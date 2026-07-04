#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I008_010 0x80
#define FSPEC02_I008_000 0x40
#define FSPEC03_I008_020 0x20
#define FSPEC04_I008_036 0x10
#define FSPEC05_I008_034 0x08
#define FSPEC06_I008_040 0x04
#define FSPEC07_I008_050 0x02

#define FSPEC08_I008_090 0x80
#define FSPEC09_I008_100 0x40
#define FSPEC10_I008_110 0x20
#define FSPEC11_I008_120 0x10
#define FSPEC12_I008_038 0x08
#define FSPEC13_I008_SP  0x04
#define FSPEC14_I008_RFS 0x02


#ifndef MIN
#define MIN (a,b) ((a<b)?(a):(b))
#endif
#ifndef MAX
#define MAX (a,b) ((a>b)?(a):(b))
#endif
#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif
#ifndef INT8_MAX
#define INT8_MAX (127)
#endif
#ifndef INT8_MIN
#define INT8_MIN (-127)
#endif



typedef unsigned time_msec;

static void     send_i008_020 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);
static bool     send_i008_034 (uint8_t * fspec, uint8_t * &field,
                               RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems);
static bool     send_i008_036 (uint8_t * fspec, uint8_t * &field,
                               RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems);
static bool     send_i008_040 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather, unsigned maxsize, unsigned &WrittenNumberOfItems);
static bool     send_i008_050 (uint8_t * fspec, uint8_t * &field,
                               RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems);
static bool     send_i008_038 (uint8_t * fspec, uint8_t * &field,
                               RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems);
static void     send_i008_090 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);
static void     send_i008_100 (uint8_t * fspec, uint8_t * &field, const int scaling_factor);
//static void send_i008_110 (uint8_t * fspec, uint8_t * &field);
static void     send_i008_120 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);

//bool is_intermediate_contour = false;
//unsigned contour_sn = 0;
//const unsigned  c_BufferSize = 256;     // velikost paketu, viz EGIS-SDPRL ed. 2.0., RDPRL_112

bool Asterix008_to_WeatherMessage (const void *data, int size,
                                   deque < RDFWeatherMessage > &weather_list, int &scaling_factor, std::stringstream * out_str, bool one_line)
{
  uint8_t        *msg;
  uint8_t        *record;
  uint8_t        *fspec;
  uint8_t        *field;
  int             cat;
  int             len;
  int             key_scaling = 0;
  static          map < unsigned, int >scaling_factor_map;
  double          scaling_factor_polar;
  double          scaling_factor_cartesian;
  double          vector_orientation;

  weather_list.clear ();

  if (!data)
  {
    cms_error ("wrong data");
    return false;
  }

  if (size < 0 || size > 0xffff)
  {
    cms_error ("wrong size %d", size);
    return false;
  }

  msg = (uint8_t *) data;
  cat = msg[0];
  len = (((unsigned)msg[1]) << 8) | msg[2];
  record = msg + 3;

  if (cat != 8)
  {
    cms_error ("unknown category %d", cat);
    goto error;
  }

  if (len < size)
  {
    cms_warning ("wrong length %d %d", len, size);
    size = len;
  } else if (len > size)
  {
    cms_error ("wrong length %d %d", len, size);
    goto error;
  }

  while (record < msg + len)
  {
    // vytvorime novou polozku
    RDFWeatherMessage weather;
    scaling_factor_polar = 0;
    scaling_factor_cartesian = 0;
    vector_orientation = 0;

    // najdeme zacatek datovych poli
    fspec = field = record;
    while (*field & FSPEC_FX)
    {
      if (++field >= msg + len)
      {
        cms_error ("end of message (%d >= %d)", field - msg, len);
        goto error;
      }
    }
    field++;

    // data source identifier (2)
    if (*fspec & FSPEC01_I008_010)
    {
      if (out_str)
      {
        *out_str << "010 [sac=" << static_cast < int >(field[0]) << " sic=" << static_cast < int >(field[1]) << "]";
        NEW_LINE (*out_str, 0, '\0');
      }

      weather.SAC = field[0];
      weather.SIC = field[1];
      field += 2;

      key_scaling = (weather.SAC << 8) & weather.SIC;

      if (scaling_factor_map.find (key_scaling) != scaling_factor_map.end ())
      {
        scaling_factor = scaling_factor_map[key_scaling];
        scaling_factor_polar = exp2 (scaling_factor - 7) * 1852;
        scaling_factor_cartesian = exp2 (scaling_factor - 6) * 1852;
      } else
      {
        scaling_factor = 0;
        scaling_factor_polar = 0;
        scaling_factor_cartesian = 0;
      }
    }
    // message type (1)
    if (*fspec & FSPEC02_I008_000)
    {
      if (out_str)
      {
        switch (field[0])
        {
          case 1:
            *out_str << "000 [type=polar vector]";
            break;
          case 2:
            *out_str << "000 [type=cartesian vector_1]";
            break;
          case 4:
            *out_str << "000 [type=cartesian vector_2]";
            break;
          case 3:
            *out_str << "000 [type=contour record]";
            break;
          case 255:
            *out_str << "000 [type=EOP]";
            break;
          case 254:
            *out_str << "000 [type=SOP]";
            break;
          default:
            *out_str << "000 [unknown type=" << static_cast < int >(field[0]) << "]";
            break;
        }
        NEW_LINE (*out_str, 0, '\0');
      }

      switch (field[0])
      {
        case 1:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_POLAR_VECTOR;
            break;
          }
        case 2:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_1;
            break;
          }
        case 3:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_CONTOUR_RECORD;
            break;
          }
        case 4:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_2;
            break;
          }
        case 254:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_START_OF_PICTURE;
            break;
          }
        case 255:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_END_OF_PICTURE;
            break;
          }
        default:
          {
            cms_error ("unknown message type");
            goto error;
          }
      }

      field += 1;
    }
    // vector qualifier (1+)
    if (*fspec & FSPEC03_I008_020)
    {
      weather.VectorSystemCoordinates = (field[0] & 0x80) ? true : false;
      weather.VectorIntensity = (field[0] >> 4) & 7;
      vector_orientation = (double)((field[0] >> 1) & 7) * (M_PI / 8);

      if (out_str)
      {
        *out_str << "020 [";
        if (weather.VectorSystemCoordinates)
          *out_str << "org=system coords ";
        else
          *out_str << "org=local coords ";

        *out_str << "intensity=" << static_cast < int >(weather.VectorIntensity);
        *out_str << " heading=" << ((double) ((field[0] >> 1) & 7) * (22.5));
        *out_str << " test_vector=" << static_cast < int >(weather.TestVector);
        *out_str << "]";

        NEW_LINE (*out_str, 0, '\0');
      }

      if (field[0] & 1)
      {
        field++;
        weather.TestVector = (field[0] >> 2) & 1;
        //bool error_condition = (field[0] >> 1) & 1;
      }

      if (field[0] & 1)
      {
        cms_error ("");
        goto error;
      }

      field++;
    }
    // sequence of cartesian vectors in SPF notation (1 + 3 * n)
    if (*fspec & FSPEC04_I008_036)
    {
      unsigned        rep = field[0];
      field++;

      if (out_str)
      {
        *out_str << "036 [sequence_of `cartesian vectors', rep=" << rep << "]";
      }

      for (unsigned i = 0; i < rep; i++)
      {
        if (scaling_factor_cartesian != 0)
        {
          RDFWeatherCartesianVector vector;
          vector.X = (double)((char)field[0]) * scaling_factor_cartesian;
          vector.Y = (double)((char)field[1]) * scaling_factor_cartesian;
          vector.Length = (double)field[2] * scaling_factor_cartesian;
          vector.Azimuth = vector_orientation;

          if (out_str)
          {
            NEW_LINE (*out_str, 4, ' ');

            vector.to_stringstream (*out_str, one_line);
          }

          weather.CartesianVector.push_back (vector);
        }
        field += 3;
      }

      if (out_str)
      {
        NEW_LINE (*out_str, 0, '\0');
      }
    }
    // sequence of polar vectors in SPF notation (1 + 4 * n)
    if (*fspec & FSPEC05_I008_034)
    {
      unsigned        rep = field[0];
      field++;
      if (out_str)
      {
        *out_str << "034 [sequence_of `polar vectors', rep=" << rep << "]";
      }


      for (unsigned i = 0; i < rep; i++)
      {
        RDFWeatherPolarVector vector;
        vector.StartRange = field[0] * scaling_factor_polar;
        vector.EndRange = field[1] * scaling_factor_polar;
        vector.Azimuth = field2unsigned (field + 2, 16) * 2 * M_PI / 0x10000;

        if (out_str)
        {
          NEW_LINE (*out_str, 4, ' ');

          vector.to_stringstream (*out_str, one_line);
        }


        if (scaling_factor_polar != 0)
        {
          weather.PolarVector.push_back (vector);
        }

        field += 4;
      }

      if (out_str)
      {
        NEW_LINE (*out_str, 0, '\0');
      }

    }
    // contour identifier (2)
    if (*fspec & FSPEC06_I008_040)
    {
      weather.ContourSystemCoordinates = (field[0] & 0x80) ? true : false;
      weather.ContourIntensity = (field[0] >> 4) & 7;

      switch (field[0] & 3)
      {
        case 0:
          {
            weather.ContourType = WEATHER_CONTOUR_TYPE_INTERMEDIATE;
            break;
          }
        case 1:
          {
            weather.ContourType = WEATHER_CONTOUR_TYPE_LAST;
            break;
          }
        case 2:
          {
            weather.ContourType = WEATHER_CONTOUR_TYPE_FIRST;
            break;
          }
        case 3:
          {
            weather.ContourType = WEATHER_CONTOUR_TYPE_FIRST_AND_ONLY;
            break;
          }
      }

      weather.ContourSN = field[1];

      if (out_str)
      {
        *out_str << "040 [";
        if (weather.ContourSystemCoordinates)
          *out_str << "org=system coords ";
        else
          *out_str << "org=local coords ";

        *out_str << "intensity=" << static_cast < int >(weather.ContourIntensity);
        *out_str << " fst/lst=";
        switch (weather.ContourType)
        {
          case WEATHER_CONTOUR_TYPE_INTERMEDIATE:
            *out_str << "intermediate_record";
            break;
          case WEATHER_CONTOUR_TYPE_LAST:
            *out_str << "last_record";
            break;
          case WEATHER_CONTOUR_TYPE_FIRST:
            *out_str << "first_record";
            break;
          case WEATHER_CONTOUR_TYPE_FIRST_AND_ONLY:
            *out_str << "first_and_only_record";
            break;
          case WEATHER_CONTOUR_TYPE_UNKNOWN:
            *out_str << "unknown";
            break;
        }
        *out_str << " sn=" << static_cast < int >(weather.ContourSN) << "]";

        NEW_LINE (*out_str, 0, '\0');
      }
      field += 2;
    }
    // sequence of contour points in SPF notation (1 + 2 * n)
    if (*fspec & FSPEC07_I008_050)
    {
      unsigned        rep = field[0];
      field++;

      if (out_str)
      {
        *out_str << "050 [sequence_of `countour points', rep=" << rep << "]";
      }

      for (unsigned i = 0; i < rep; i++)
      {
        if (scaling_factor_cartesian != 0)
        {
          RDFWeatherContourPoint point;
          point.X = (double)((char)field[0]) * scaling_factor_cartesian;
          point.Y = (double)((char)field[1]) * scaling_factor_cartesian;

          if (out_str)
          {
            NEW_LINE (*out_str, 4, ' ');

            point.to_stringstream (*out_str, one_line);
          }


          weather.ContourPoint.push_back (point);
        }
        field += 2;
      }

      if (out_str)
      {
        NEW_LINE (*out_str, 0, '\0');
      }
    }

    if (!(*fspec & FSPEC_FX))
    {
      goto end;
    }

    fspec++;

    // time of day (3)
    if (*fspec & FSPEC08_I008_090)
    {
      if (out_str)
      {
        uint64_t        time = field2unsigned (field, 24);

        uint64_t        mod = time % 128;
        time = time / 128;
        *out_str << "070 [time=" << time << " ";
        *out_str << std::setw (2) << std::setfill ('0') << time / 3600  //h
          << ":" << std::setw (2) << std::setfill ('0') << ((time % 3600) / 60) << ":" << std::setw (2) //m
          << std::setfill ('0') << ((time % 3600) % 60) //s
        << "." << std::setw (3) << int (1000 * mod * (1.0 / 128.0))     //ms
        <<              "]";

        NEW_LINE (*out_str, 0, '\0');
      }

      weather.Time.Present = true;
      weather.Time.Time = ((long long)field2unsigned (field, 24) * 1000 + 64) / 128;
      field += 3;
    }
    // processing status (3+)
    if (*fspec & FSPEC09_I008_100)
    {
      scaling_factor = ((char)field[0]) >> 3;
      scaling_factor_map[key_scaling] = scaling_factor;
      scaling_factor_polar = exp2 (scaling_factor - 7) * 1852;
      scaling_factor_cartesian = exp2 (scaling_factor - 6) * 1852;

      if (out_str)
      {
        scaling_factor = ((char)field[0]) >> 3;
        unsigned short  reduction_stage = field[0] & 7;
        unsigned short  processing_parameters = field2unsigned (&field[1], 15);

        *out_str << "100 [f=" << scaling_factor << " R=" << reduction_stage << " Q=" << processing_parameters << "]";
      }
      //cms_info ("scaling factor %x %d", field[0], scaling_factor);
      //uint8_t reduction_stage = field[0] & 7;
      //unsigned short processing_parameters = bytes2 (field[2], field[1]) >> 1;
      if (field[2] & 1)
      {
        field++;
      }
      field += 3;
    }
    // station configuration status (1+)
    if (*fspec & FSPEC10_I008_110)
    {
      if (field[0] & 1)
      {
        field++;
      }
      field++;
    }
    // total number of items constituting on weather picture (2)
    if (*fspec & FSPEC11_I008_120)
    {
      if (out_str)
      {
        *out_str << "120 [total_number_of_items=" << field2unsigned (field, 16) << "]";

        NEW_LINE (*out_str, 0, '\0');
      }

      weather.TotalNumberOfItems = field2unsigned (field, 16);
      field += 2;
    }
    // sequence of weather vectors in SPF notation (1 + 4 * n)
    if (*fspec & FSPEC12_I008_038)
    {
      unsigned        rep = field[0];
      field++;

      if (out_str)
      {
        *out_str << "038 [sequence_of `weather vectors', rep=" << rep << "]";
      }


      for (unsigned i = 0; i < rep; i++)
      {
        if (scaling_factor_cartesian != 0)
        {
          RDFWeatherWeatherVector vector;
          vector.X1 = (double)((char)field[0]) * scaling_factor_cartesian;
          vector.Y1 = (double)((char)field[1]) * scaling_factor_cartesian;
          vector.X2 = (double)((char)field[2]) * scaling_factor_cartesian;
          vector.Y2 = (double)((char)field[3]) * scaling_factor_cartesian;

          if (out_str)
          {
            NEW_LINE (*out_str, 4, ' ');

            vector.to_stringstream (*out_str, one_line);
          }

          weather.WeatherVector.push_back (vector);
        }
        field += 4;
      }

      if (out_str)
      {
        NEW_LINE (*out_str, 0, '\0');
      }

    }

    if (*fspec & FSPEC13_I008_SP)
    {
      field += field[0];
    }

    if (*fspec & FSPEC14_I008_RFS)
    {
      field += field[0];
    }

    if (!(*fspec & FSPEC_FX))
    {
      goto end;
    }

    fspec++;

    cms_error ("unable to handle more fspec");
    goto error;

  end:
    // pridame polozku do seznamu
    weather_list.push_back (weather);

    // presuneme se na dalsi zaznam
    record = field;
  }

  if (record > msg + len)
  {
    cms_error ("wrong packet length %d > %d", record - msg, size);
    goto error;
  }

  return true;

error:
  cms_data (size, msg);
  weather_list.clear ();
  return false;
}


int WeatherMessage_to_Asterix008 (deque < RDFWeatherMessage > &weather_list, void *data, int maxsize, int scaling_factor, unsigned &WrittenNumberOfItems)
{
  uint8_t         buffer[4096];
  uint8_t        *msg;
  uint8_t        *record;
  uint8_t        *fspec;
  uint8_t        *field;
  int             fspec_size = 0;
  int             field_size = 0;
  int             size;
  bool            is_act_message_vector_empty;

  memset (data, 0, maxsize);
  msg = (uint8_t *) data;
  msg[0] = 8;                   // kategorie
  msg[1] = 0;                   // delka
  msg[2] = 0;

  // seznam je prazdny, neni co zpracovavat
  if (weather_list.empty ())
  {
    return 0;
  }
  size = 3;

  bool exit_conversion = false;
  while ((weather_list.size () > 0) && (!exit_conversion))
  {
    // tady nesmi byt RDFWeatherMessage & weather
    // je potreba vytvorit kopii prvniho prvku seznamu
    // je totiz mozne, ze na konci funkce budeme
    // pozmenenou strukturu vracet na zacatek seznamu
    RDFWeatherMessage weather = weather_list.front ();

    is_act_message_vector_empty = false;
    memset (buffer, 0, sizeof (buffer));
    record = fspec = msg + size;
    field = buffer;

    // data source identifier
    {
      *fspec |= FSPEC01_I008_010;
      field[0] = weather.SAC;
      field[1] = weather.SIC;
      field += 2;
    }
    // message type
    switch (weather.MessageType)
    {
      case WEATHER_MESSAGE_TYPE_POLAR_VECTOR:
        {
          *fspec |= FSPEC02_I008_000;
          uint8_t *orig_field = field;
          field[0] = 1;
          field += 1;
          send_i008_020 (fspec, field, weather);
          unsigned size_avail = maxsize - fspec_size - field_size - size - 10;
          unsigned origWrittenNumberOfItems = WrittenNumberOfItems;
          is_act_message_vector_empty = send_i008_034 (fspec, field, weather, scaling_factor, size_avail, WrittenNumberOfItems);
          if (origWrittenNumberOfItems == WrittenNumberOfItems) { field = orig_field; }
          break;
        }
      case WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_1:
        {
          *fspec |= FSPEC02_I008_000;
          uint8_t *orig_field = field;
          field[0] = 2;
          field += 1;
          send_i008_020 (fspec, field, weather);
          unsigned origWrittenNumberOfItems = WrittenNumberOfItems;
          unsigned size_avail = maxsize - fspec_size - field_size - size - 10;
          is_act_message_vector_empty = send_i008_036 (fspec, field, weather, scaling_factor, size_avail, WrittenNumberOfItems);
          if (origWrittenNumberOfItems == WrittenNumberOfItems) { field = orig_field; }
          break;
        }
      case WEATHER_MESSAGE_TYPE_CONTOUR_RECORD:
        {
          *fspec |= FSPEC02_I008_000;
          field[0] = 3;
          field += 1;

          unsigned size_avail = maxsize - fspec_size - field_size - size - 10;
          bool max_sn_reached = send_i008_040 (fspec, field, weather, size_avail, WrittenNumberOfItems);
          fspec_size = fspec - record + 1;
          field_size = field - buffer;

          size_avail = maxsize - fspec_size - field_size - size - 10;
          is_act_message_vector_empty = send_i008_050 (fspec, field, weather, scaling_factor, size_avail, WrittenNumberOfItems);

          if (max_sn_reached)
          {
            is_act_message_vector_empty = true;
            weather.ContourPoint.clear ();
          }
          break;
        }
      case WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_2:
        {
          *fspec |= FSPEC02_I008_000;
          uint8_t *orig_field = field;
          field[0] = 4;
          field += 1;
          send_i008_020 (fspec, field, weather);
          *fspec |= FSPEC_FX;
          fspec++;
          unsigned origWrittenNumberOfItems = WrittenNumberOfItems;
          unsigned size_avail = maxsize - fspec_size - field_size - size - 10;
          is_act_message_vector_empty = send_i008_038 (fspec, field, weather, scaling_factor, size_avail, WrittenNumberOfItems);
          if (origWrittenNumberOfItems == WrittenNumberOfItems) { field = orig_field; }
          break;
        }
      case WEATHER_MESSAGE_TYPE_START_OF_PICTURE:
        {
          *fspec |= FSPEC02_I008_000;
          field[0] = 254;
          field += 1;
          *fspec |= FSPEC_FX;
          fspec++;
          if (weather.Time.Present)
          {
            send_i008_090 (fspec, field, weather);
          }
          send_i008_100 (fspec, field, scaling_factor);
          // send_i008_110(fspec, field);
          is_act_message_vector_empty = true;
          //is_intermediate_contour = false;
          //contour_sn = 0;
          break;
        }
      case WEATHER_MESSAGE_TYPE_END_OF_PICTURE:
        {
          *fspec |= FSPEC02_I008_000;
          field[0] = 255;
          field += 1;
          *fspec |= FSPEC_FX;
          fspec++;
          if (weather.Time.Present)
          {
            send_i008_090 (fspec, field, weather);
          }
          // send_i008_110(fspec, field);
          send_i008_120 (fspec, field, weather);
          is_act_message_vector_empty = true;
          break;
        }
      default:
        {
          cms_error ("unknown message type");
        }
    }

    // vymazeme nevyuzite polozky fspec
    while (!(*fspec & ~FSPEC_FX))
    {
      fspec--;
    }

    *fspec &= ~FSPEC_FX;

    // konec polozky fspec
    fspec++;

    // zkontrolujeme, jestli se polozka vejde do zaznamu
    fspec_size = fspec - record;
    field_size = field - buffer;
    if (size + fspec_size + field_size > maxsize)
    {
      if (size == 3)
      {
        cms_error ("not enough space in output buffer (size=%d , fspec_size=%d , field_size=%d, maxsize=%d) weatherlist.size()=%d",size , fspec_size , field_size, maxsize,weather_list.size());
//        cms_error ("not enough space in output buffer (size=%d , fspec_size=%d , field_size=%d, maxsize=%d)",size , fspec_size , field_size, maxsize);
        weather_list.pop_front ();

// toto musime navzdy zahodit
      } else {
        exit_conversion = true; // uz se nic dalsiho nevejde
        // nevejde se, zahodi se, nemelo by se stavat
        weather_list.pop_front (); weather_list.push_front (weather);  // update !!!!???
      }
    } else {
      // zkopirujeme polozku do zaznamu
      memcpy (fspec, buffer, field_size);
      size += fspec_size + field_size;
      if (is_act_message_vector_empty)
      {
        // odstranime strukturu z fronty
        weather_list.pop_front ();
      } else {
        exit_conversion = true;
        weather_list.pop_front (); weather_list.push_front (weather);  // update !!!!???
      }
    }
  }

  if (size <= 7) // CAT, LEN1, LEN2 [, FSPEC, SAC, SIC]
  {
    return 0;
  }

  msg[1] = (size >> 8) & 0xff;
  msg[2] = size & 0xff;
  return size;
}

// Vector Qualifier
void send_i008_020 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC03_I008_020;
  field[0] |= weather.VectorSystemCoordinates ? 0x80 : 0;
  field[0] |= (weather.VectorIntensity & 7) << 4;
//  printf("\n----- INTENSITY %d -----\n",weather.VectorIntensity);
  double          vector_orientation = weather.CartesianVector.size ()? weather.CartesianVector[0].Azimuth : 0;
  field[0] |= ((uint8_t) rint (vector_orientation / (M_PI / 8)) & 7) << 1;
  if (weather.TestVector)
  {
    field[0] |= FSPEC_FX;
    field++;
    field[0] |= 1 << 2;
  }
  field++;
}

// vraci stav, zda se podarilo vycist cely vector;
// Sequence of Polar Vectors in SPF Notation
bool send_i008_034 (uint8_t * fspec, uint8_t * &field, RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems)
{
  unsigned        max_message_items = MIN(maxsize / 4, UINT8_MAX);   // cat(1),size(2),fspec(1),010(2),000(1),020(1+),038(1+4*n)
  unsigned        rep = weather.PolarVector.size ();
  unsigned int    un_azimuth;
  bool            bRet = false;
  uint8_t        *p_size;
  double          local_scaling_factor = exp2 (scaling_factor - 7);

  *fspec |= FSPEC05_I008_034;
  p_size = field;
//  *p_size = weather.PolarVector.size () < max_message_items ? weather.PolarVector.size () : max_message_items;
  *p_size = 0;
  field++;
  for (unsigned i = 0; i < rep; i++)
  {
    if (i >= max_message_items)
    {
      cms_debug ("max_items_exceed [rep=%d]", rep);
      return bRet;
    }
    RDFWeatherPolarVector vec = weather.PolarVector.front ();
    weather.PolarVector.pop_front ();
//    if (vec.EndRange - vec.StartRange < NM_TO_METER * local_scaling_factor)
//    {
//      (*p_size)--;
 //     continue;
 //   }

    bool            usable = true;
    double          d_start_scaled = vec.StartRange / NM_TO_METER / local_scaling_factor;
    double          d_end_scaled   = vec.EndRange   / NM_TO_METER / local_scaling_factor;

    if (d_start_scaled < 0.0) d_start_scaled = 0.0;
    if (d_end_scaled   < 0.0) d_end_scaled   = 0.0;
    if (d_start_scaled > UINT8_MAX) d_start_scaled = UINT8_MAX;
    if (d_end_scaled   > UINT8_MAX) d_end_scaled   = UINT8_MAX;
    if (d_end_scaled <= d_start_scaled) usable = false;

    uint8_t        start_scaled = (uint8_t) d_start_scaled;
    uint8_t        end_scaled = (uint8_t) d_end_scaled;
    if (end_scaled <= start_scaled) usable = false;

    if (usable)
    {
      un_azimuth = (unsigned int)rint (vec.Azimuth / 2. / M_PI * 0x10000);
      field[0] = start_scaled;
      field[1] = end_scaled;
      field[2] = (uint8_t) ((un_azimuth & 0xFF00) >> 8);
      field[3] = (uint8_t) (un_azimuth & 0x00FF);
      field += 4;
      WrittenNumberOfItems++;
      (*p_size)++;
//    } else {
//      printf("  REMOVED\n");
    }
  }
  bRet = true;

  return bRet;
}

// Sequence of Cartesian Vectors in SPF Notation
bool send_i008_036 (uint8_t * fspec, uint8_t * &field, RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems)
{
  unsigned        max_message_items =  MIN(maxsize / 3, UINT8_MAX);  // cat(1),size(2),fspec(1),010(2),000(1),020(1+),038(1+3*n)
  if (max_message_items>255) max_message_items=255; /// because REP is one byte
  unsigned        rep = weather.CartesianVector.size ();
  uint8_t        *p_size;
  double          local_scaling_factor = exp2 (scaling_factor - 6);

  *fspec |= FSPEC04_I008_036;
  p_size = field;
  *p_size = 0;
  field++;
  for (unsigned i = 0; i < rep; i++)
  {
    if ((*p_size) >= max_message_items)
    {
      cms_debug ("max_items_exceed [rep=%d]", rep);
      return false;
    }
    RDFWeatherCartesianVector vec = weather.CartesianVector.front ();
    weather.CartesianVector.pop_front ();
    if (vec.Length < NM_TO_METER * local_scaling_factor)
    {
      vec.Length = NM_TO_METER * local_scaling_factor;
      //(*p_size)--; // Pokud se vyhodi, jako v pripade polarnich souradnic, tak jsou v obr velke diry. Rastr je dosti hruby.
      //continue;    // Casto totiz velikost neprojde kriteriem o kousek.
    }

    bool            usable = true;
    double          d_x_scaled = vec.X / NM_TO_METER / local_scaling_factor;
    double          d_y_scaled = vec.Y / NM_TO_METER / local_scaling_factor;
    double          d_len_scaled = vec.Length / NM_TO_METER / local_scaling_factor;

//    printf(" Y=%14.3f X=%14.3f Len=%14.3f ==>> Y=%9.3f X=%9.3f Len=%9.3f ",vec.Y,vec.X,vec.Length,d_y_scaled,d_x_scaled,d_len_scaled);
    if ((d_y_scaled < INT8_MIN) || (d_y_scaled > INT8_MAX)) usable = false;
    if (d_x_scaled > INT8_MAX) usable = false;
    if (usable)
    {
      if (d_x_scaled < INT8_MIN)
      {
        d_len_scaled += (d_x_scaled - INT8_MIN);
        d_x_scaled = INT8_MIN;
      }
      if ((d_len_scaled + d_x_scaled) > INT8_MAX)  d_len_scaled = (INT8_MAX-1) - d_x_scaled;
      if (d_len_scaled <= 0.0) usable = false;
    }

    int8_t        x_scaled = (int8_t) d_x_scaled;
    int8_t        y_scaled = (int8_t) d_y_scaled;
    int8_t        len_scaled = (int8_t) d_len_scaled;

    if (len_scaled == 0) usable = false;

    if (usable)
    {
      field[0] = x_scaled;
      field[1] = y_scaled;
      field[2] = len_scaled;
      field += 3;
      WrittenNumberOfItems++;
      (*p_size)++;
//      printf(" >> Y=%9.3f X=%9.3f LEN=%9.3f",d_y_scaled,d_x_scaled,d_len_scaled);
//      printf(" >> Y=%03d X=%03d LEN=%03d\n",y_scaled,x_scaled,len_scaled);
//    } else {
//      printf(" ... REMOVED\n");
    }
  }

  return true;
}

// Contour Identifier
bool send_i008_040 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather, unsigned maxsize, unsigned &WrittenNumberOfItems)
{
  *fspec |= FSPEC06_I008_040;

  field[0] = weather.ContourSystemCoordinates ? 0x80 : 0;
  field[0] |= (weather.ContourIntensity & 7) << 4;
  field[0] &= 0xf0;             // vynulovany Spare bity
  switch (weather.ContourType)
  {
    case WEATHER_CONTOUR_TYPE_UNKNOWN:
      {
        cms_warning ("unknown contour type");
        break;
      }
    case WEATHER_CONTOUR_TYPE_INTERMEDIATE:
      {
        field[0] |= 0x00;
        break;
      }
    case WEATHER_CONTOUR_TYPE_LAST:
      {
        field[0] |= 0x01;
        break;
      }
    case WEATHER_CONTOUR_TYPE_FIRST:
      {
        field[0] |= 0x02;
        break;
      }
    case WEATHER_CONTOUR_TYPE_FIRST_AND_ONLY:
      {
        field[0] |= 0x03;
        break;
      }
  }
  field[1] = weather.ContourSN;

  field += 2;

  return true;
}

// Sequence of Contour Points in SPF Notation
bool send_i008_050 (uint8_t * fspec, uint8_t * &field, RDFWeatherMessage & weather,
                    const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems)
{
  unsigned        max_items_amount =  MIN((maxsize - 2) / 2, UINT8_MAX);  // max pocet poslanych bodu, odecten rep a fspec
  unsigned        items_amount = weather.ContourPoint.size ();

  if (items_amount > max_items_amount)
  {
    cms_error ("number of items [%d] reduced to [%d]\n", items_amount, max_items_amount);
    items_amount = max_items_amount;
  }

  *fspec |= FSPEC07_I008_050;
  field[0] = items_amount;      // REP
  field++;

  for (unsigned i = 0; i < items_amount; i++)
  {
    if (true)
    {
      RDFWeatherContourPoint point = weather.ContourPoint.front ();
      field[0] = (uint8_t) (point.X / NM_TO_METER / exp2 (scaling_factor - 6));
      field[1] = (uint8_t) (point.Y / NM_TO_METER / exp2 (scaling_factor - 6));
      weather.ContourPoint.pop_front ();
      field += 2;
      WrittenNumberOfItems++;
//    } else {
//      printf("unusable item\n");
    }
  }

  return true;
}


// Sequence of Weather Vectors in SPF Notation
bool send_i008_038 (uint8_t * fspec, uint8_t * &field, RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems)
{
  unsigned        max_message_items = MIN(maxsize / 4, UINT8_MAX); // cat(1),size(2),fspec(2),010(2),000(1),020(1+),038(1+4*n)
  if (max_message_items>255) max_message_items=255; /// because REP is one byte
  unsigned        rep = weather.WeatherVector.size ();
  uint8_t        *p_size;
  double          local_scaling_factor = exp2 (scaling_factor - 6);

  *fspec |= FSPEC12_I008_038;
  p_size = field;
//  *p_size = (weather.WeatherVector.size () < max_message_items) ? weather.WeatherVector.size () : max_message_items;
  *p_size = 0;
  field++;
  for (unsigned i = 0; i < rep; i++)
  {
    if ((*p_size) >= max_message_items)
    {
      cms_debug ("max_items_exceed [rep=%d]", rep);
      return false;
    }
    RDFWeatherWeatherVector vec = weather.WeatherVector.front ();
    weather.WeatherVector.pop_front ();
    // double dx = fabs(vec.X1 - vec.X2);
    // double dy = fabs(vec.Y1 - vec.Y2);
    // double length = sqrt(dx*dx +dy*dy);
    // if (length < NM_TO_METER*local_scaling_factor) {
    //       vec.Length = NM_TO_METER*local_scaling_factor;
    //       (*p_size)--; // Pokud se vyhodi, jako v pripade polarnich souradnic, tak jsou v obr velke diry. Rastr je dosti hruby.
    //       continue;    // Casto totiz velikost neprojde kriteriem o kousek.
    // }

    bool            usable = true;
    double          d_x1_scaled = vec.X1 / NM_TO_METER / local_scaling_factor;
    double          d_y1_scaled = vec.Y1 / NM_TO_METER / local_scaling_factor;
    double          d_x2_scaled = vec.X2 / NM_TO_METER / local_scaling_factor;
    double          d_y2_scaled = vec.Y2 / NM_TO_METER / local_scaling_factor;
//    printf(" Y1=%14.3f X1=%14.3f Y2=%14.3f X2=%14.3f ==>> Y1=%9.3f X1=%9.3f Y2=%9.3f X2=%9.3f  ",vec.Y1,vec.X1,vec.Y2,vec.X2,d_y1_scaled,d_x1_scaled,d_y2_scaled,d_x2_scaled);
    if (d_y1_scaled == d_y2_scaled) // i can chack only horizontal weather vectors
    {
      double  d_len_scaled = d_x2_scaled - d_x1_scaled;
      if ((d_y1_scaled < INT8_MIN) || (d_y1_scaled > INT8_MAX)) usable = false;
      if (d_x1_scaled > INT8_MAX) usable = false;
      if (usable)
      {
        if (d_x1_scaled > INT8_MAX) usable = false;
        if (usable)
        {
          if (d_x1_scaled < INT8_MIN)
          {
            d_len_scaled += d_x1_scaled - INT8_MIN;
            d_x1_scaled = INT8_MIN;
          }
          if ((d_len_scaled + d_x1_scaled) > INT8_MAX)  d_len_scaled = (INT8_MAX-1) - d_x1_scaled;
          if (d_len_scaled <= 0.0) usable = false;
        }
        if (usable)
        {
          d_x2_scaled = d_x1_scaled + d_len_scaled;
        }
      }
    }

    int8_t        x1_scaled = (int8_t) d_x1_scaled;
    int8_t        y1_scaled = (int8_t) d_y1_scaled;
    int8_t        x2_scaled = (int8_t) d_x2_scaled;
    int8_t        y2_scaled = (int8_t) d_y2_scaled;
    if (usable)
    {
      field[0] = x1_scaled;
      field[1] = y1_scaled;
      field[2] = x2_scaled;
      field[3] = y2_scaled;
      field += 4;
      WrittenNumberOfItems++;
      (*p_size)++;
//      printf(" >> Y1=%9.3f X1=%9.3f Y2=%9.3f X2=%9.3f",d_y1_scaled,d_x1_scaled,d_y2_scaled,d_x2_scaled);
//      printf(" >> Y1=%03d X1=%03d Y2=%03d X2=%03d \n",y1_scaled,x1_scaled,y2_scaled,x2_scaled);
//    } else {
//      printf(" ... REMOVED\n");
    }
  }

  return true;

}

// Time of Day
void send_i008_090 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC08_I008_090;
  int             time = RDF_time_to_asterix_time(weather.Time.Time);
  field[0] = (time >> 16) & 0xff;
  field[1] = (time >> 8) & 0xff;
  field[2] = time & 0xff;
  field += 3;

}

// Processing Status
void send_i008_100 (uint8_t * fspec, uint8_t * &field, const int scaling_factor)
{
  *fspec |= FSPEC09_I008_100;
  uint8_t         R = (0 & 0x03);       // reduction stage, normal operation = 0, Apps dependent
  field[0] = (scaling_factor & 0x1f) << 3;
  field[0] |= R;
  field[1] = 0;                 // Processing parameters. Apps dependent
  field[2] = 0;
  field += 3;
}

// Station Configuration Status
// void send_i008_110 (uint8_t * fspec, uint8_t * &field)
// {
//      *fspec |= FSPEC10_I008_110;
//      field[0] = 0;
//      field += 1;
// }

// Total Number of Items Contituting One Weather Picture
void send_i008_120 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC11_I008_120;
  field[0] = (weather.TotalNumberOfItems & 0xFF00) >> 8;
  field[1] = (weather.TotalNumberOfItems & 0x00FF);
  field += 2;
}
