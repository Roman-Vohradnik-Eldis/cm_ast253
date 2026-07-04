#include "cm_send_buffer_simple.hh"
#include "cm_debug.hh"
#include "cm_send_buffer_state.hh"
#include "cm_utils.hh"
#include "compress_base.hh"
#include "compress_none.hh"

#ifdef USE_LIB_ZSTD

#include "compress_zstd.hh"
std::unique_ptr<CompressBase> make_compressor(int level)
{
    return std::unique_ptr<Compress_zstd>(new Compress_zstd(level));
}

#else

std::unique_ptr<CompressBase> make_compressor(int)
{
    throw std::runtime_error("Cannot use compression, compile LibCM with zstd support");
}

#endif // USE_LIB_ZSTD


SendBufferSimple::SendBufferSimple(SendBufferSimpleSettings const &settings)
    : use_control_character(settings.use_control_character),
      use_compression(settings.compression_level)
{
    CHECK(!use_compression || use_control_character, "You must use control character when compression is enabled");
    if (settings.compression_level)
        compress = make_compressor(*settings.compression_level);
    else
        compress.reset(new Compress_none());

    if (settings.debug_file_name)
        debug_file = open_shared_file(*settings.debug_file_name);
}

size_t SendBufferSimple::get_max_size() const
{
    // Odecteme znak 'n'/'t'
    assert(max_size > 0);
    return max_size - 1;
}

void SendBufferSimple::reset()
{
    begin_new_frame = true;
    compress->reset();
}

void SendBufferSimple::clear()
{
    compress->get().clear();
}

void SendBufferSimple::set_max_size(size_t size)
{
    if (size < 32)
    {
        // Tohle nevim, jestli je dobre ignorovat pozadavek, ale je
        // potreba aby se tam vesla aspon zstd hlavicka, jinak by
        // nikdy try_add_message neuspelo.
        cms_ns_if_print("libcm", 1, "Cannot set max buffer size set to %zu, using %u",
                        size, 32);
        size = 32;
    }
    max_size = size;
}

std::string change_nonprintable(View<uint8_t> in)
{
    std::string result(in.begin(), in.end());
    for (char &c : result)
        if (!isgraph(c) && c != ' ')
            c = '?';
    return result;
}

ssize_t SendBufferSimple::try_add_message(char partial_kind,
                                          char complete_kind,
                                          View<uint8_t> msg_header,
                                          View<uint8_t> body,
                                          size_t send_at_least)
{
    if (!compress->get().empty())
    {
        cms_ns_if_print("simple", 5, "try_add_message: %c/%c: Message already present", partial_kind, complete_kind);
        return -1;
    }

    // +1 : pridava se znak 't' kvuli zpetne kompatibilite
    size_t complete_header_size = msg_header.size() + 1 + (use_control_character ? 1 : 0);
    
    if (complete_header_size > max_size)
    {
        cms_ns_if_print("simple", 5, "try_add_message: %c/%c: Header do not fit", partial_kind, complete_kind);
        return -1;
    }
    
    size_t size_to_add = std::min(body.size(), max_size - complete_header_size);
    if (use_compression)
    {
        size_to_add = last_size_to_add;
        // size_to_add = 4096;
    }

    for (;;)
    {
        if (size_to_add < send_at_least)
        {
            // Tohle nastane asi jen pripade, ze muzeme poslat z tela
            // max treba 1 byte, ale komprese ma nejakou rezii, a nevejde se
            // tam vubec nic.
            cms_ns_if_print("simple", 5, "try_add_message %c/%c: Cannot send anything", partial_kind, complete_kind);
            return -1;
        }

        {
            ByteBuffer &b = compress->get();
            b.push_back('_'); // to be filled later
        }
        if (use_control_character)
            compress->get().push_back(use_compression ? (begin_new_frame ? 'n' : 'c') : 't');

        compress->add_data(view_bytes(msg_header.begin(), msg_header.end()),
                           body.empty() ? CompressFlushMode::Flush : CompressFlushMode::None);

        size_t n_blocks = 0;
        size_t max_out_block_size = 0;
        size_t beginning = 0;
        bool last_round = false;
        while (beginning < body.size())
        {
            size_t prev_size = compress->get().size();
            size_t input_chunk_size = std::min(body.size() - beginning, last_round ? size_to_add / 2 : size_to_add);
            compress->add_data(view_bytes(body.begin() + beginning,
                                          body.begin() + beginning + input_chunk_size),
                               CompressFlushMode::Flush);
            size_t new_size = compress->get().size();
            size_t out_block_size = new_size - prev_size;
            if (max_out_block_size < out_block_size)
                max_out_block_size = out_block_size;
            ++n_blocks;
            beginning += input_chunk_size;

            if (new_size + 2 * max_out_block_size >= max_size)
            {
                if (new_size + max_out_block_size < max_size)
                    last_round = true;
                else
                    break;
            }
        }
        begin_new_frame = false;
        size_t total_output_size = compress->get().size();

        bool ok = total_output_size <= max_size;
        cms_ns_if_print("simple", 5, "try_add_message: %s %c/%c:"
                        "Header=%zu bytes, Body=%zu, SizeToAdd=%zu NBlocks=%zu BodyPart=%zu Out=%zu",
                        ok ? "SUCCESS" : "FAILURE",
                        partial_kind, complete_kind,
                        msg_header.size(),
                        body.size(),
                        size_to_add,
                        n_blocks,
                        beginning,
                        total_output_size - complete_header_size);

        
        if (ok)
        {
            // Povedlo se
            assert(compress->get().size() >= complete_header_size);
            char used_kind = (beginning == body.size()) ? complete_kind : partial_kind;
            compress->get()[0] = used_kind;

            if (debug_file)
            {
                SyncedWriter writer(*debug_file);
                writer << format("db outsize=%6zu (max=%6zu)"
                                 " kind=%c"
                                 " header_size=%zu"
                                 " available_input=%zu"
                                 " consumed_input=%zu\n",
                                 total_output_size, max_size,
                                 used_kind,
                                 msg_header.size(),
                                 body.size(),
                                 beginning);
                writer.write((char const *)msg_header.data(), msg_header.size());
                writer.write((char const *)body.begin(), beginning);
                writer << "\n";
                writer.flush();
            }

            last_size_to_add = size_to_add;
            if (n_blocks < 6)
                last_size_to_add /= 2;
            if (n_blocks > 12)
                last_size_to_add *= 2;
            
            return beginning;
        }
        else
        {
            // Nepovedlo se, musime vsechno zahodit (ukoncime frame). V
            // pristi iteraci zkusime kratsi cast zpravy.
            begin_new_frame = true;
            compress->reset();
            last_size_to_add = 0;
            size_to_add /= 2;
        }
    }
}

ByteBuffer const &SendBufferSimple::get() const
{
    return compress->get();
}

// bool SendBufferSimple::try_add_message(View<uint8_t> msg)
// {
//     if (msg.empty())
//         return true;

//     size_t prev_input_size = input.size();
//     size_t prev_output_size = compress->get().size();
    
//     if (compress->get().empty())
//         compress->get().push_back(use_compression ? 'n' : 't');

//     compress->add_data(msg, CompressFlushMode::Flush);
//     size_t tried_output_size = compress->get().size();

//     // TODO: Vyhodit 
//     ByteBuffer output = compress->get();
    
//     size_t new_output_size;
//     bool ok;
//     if (tried_output_size > max_size)
//     {
//         // Nepovedlo se
//         compress->reset();
//         assert(compress->get().size() == 0);
//         new_output_size = 0;
//         if (!input.empty())
//         {
//             compress->get().push_back(use_compression ? 'n' : 't');
//             compress->add_data(view_bytes(input), CompressFlushMode::Flush);
//             new_output_size = compress->get().size();
//         }
        
//         // TODO: Tohle je problem:
//         assert(compress->get().size() <= max_size);
//         ok = false;
//     }
//     else
//     {
//         // Povedlo se.  Pro neprazdnou zpravu mame neprazdny vystup:
//         new_output_size = tried_output_size;
//         assert(tried_output_size > prev_output_size); 
//         input.insert(input.end(), msg.begin(), msg.end());
//         ok = true;
//     }
    
//     cms_ns_if_print("simple", 3, "try_add_message %zu bytes ("
//                     "prev_input_size=%zu, "
//                     "prev_output_size=%zu, "
//                     "tried_output_size=%zu, "
//                     "new_output_size=%zu, "
//                     "max_size=%zu, "
//                     "ok=%s"
//                     "):\n"
//                     "origo=%s\n"
//                     "outp=%s",
//                     msg.size(),
//                     prev_input_size,
//                     prev_output_size,
//                     tried_output_size,
//                     new_output_size,
//                     max_size,
//                     ok ? "true" : "false",
//                     std::string(msg.begin(), msg.end()).c_str(),
//                     change_nonprintable(view_bytes(output.begin(), output.end())).c_str());
//     assert(compress->get().empty() || compress->get().size() > 1);
//     assert(compress->get().empty() || (compress->get()[0] == 't' || compress->get()[0] == 'n'));
//     return ok;
// }
