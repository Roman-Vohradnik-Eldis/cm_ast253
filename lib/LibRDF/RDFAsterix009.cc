#include "RDFAsterix.hh"
#include "CMSDebug.hh"

#define FSPEC_FX 0x01

#define FSPEC01_I009_010 0x80
#define FSPEC02_I009_000 0x40
#define FSPEC03_I009_020 0x20
#define FSPEC04_I009_030 0x10
#define FSPEC05_I009_060 0x08
#define FSPEC06_I009_070 0x04
#define FSPEC07_I009_080 0x02

#define FSPEC08_I009_090 0x80
#define FSPEC09_I009_100 0x40


#ifndef MIN
#define MIN (a,b) ((a<b)?(a):(b))
#endif
#ifndef MAX
#define MAX (a,b) ((a>b)?(a):(b))
#endif
#ifndef UINT16_MAX
#define UINT16_MAX (65535)
#endif

#ifndef INT16_MAX
#define INT16_MAX (32766)
#endif
#ifndef INT16_MIN
#define INT16_MIN (-32766)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif


typedef unsigned time_msec;

static void     send_i009_020 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);
static bool     send_i009_030 (uint8_t * fspec, uint8_t * &field,
                               RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems);
static void     send_i009_070 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);
static void     send_i009_060 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);
static void     send_i009_080 (uint8_t * fspec, uint8_t * &field, const int scaling_factor);
static void     send_i009_090 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);
static void     send_i009_100 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather);


bool Asterix009_to_WeatherMessage (const void *data, int size,
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

  if (cat != 9)
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
    if (*fspec & FSPEC01_I009_010)
    {
      weather.SAC = field[0];
      weather.SIC = field[1];

      if (out_str)
      {
        *out_str << "010 [sac=" << static_cast < int >(field[0]) << " sic=" << static_cast < int >(field[1]) << "]";

        NEW_LINE (*out_str, 0, '\0');
      }

      field += 2;

      key_scaling = (weather.SAC << 8) & weather.SIC;

      if (scaling_factor_map.find (key_scaling) != scaling_factor_map.end ())
      {
        scaling_factor = scaling_factor_map[key_scaling];
        scaling_factor_cartesian = exp2 (scaling_factor - 6) * 1852;
      } else
      {
        scaling_factor = 0;
        scaling_factor_cartesian = 0;
      }
    }
    // message type (1)
    if (*fspec & FSPEC02_I009_000)
    {
      if (out_str)
      {

        switch (field[0])
        {
          case 2:
            {
              *out_str << "000 [type=cartesian vector]";
              break;
            }
          case 253:
            {
              *out_str << "000 [type=intermediate-update-step]";
              break;
            }
          case 254:
            {
              *out_str << "000 [type=SOP message]";
              break;
            }
          case 255:
            {
              *out_str << "000 [type=EOP message]";
              break;
            }
          default:
            {
              *out_str << "000 [type=" << static_cast < int >(field[0]) << "]";
              break;
            }
        }
        NEW_LINE (*out_str, 0, '\0');
      }

      switch (field[0])
      {
        case 2:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_1;
            break;
          }
        case 253:
          {
            weather.MessageType = WEATHER_MESSAGE_TYPE_INTERMEDIATE_UPDATE_STEP;
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
            cms_error ("unknown message type (%d)", field[0]);
            goto error;
          }
      }

      field += 1;
    }
    // vector qualifier (1)
    if (*fspec & FSPEC03_I009_020)
    {
      if (out_str)
      {
        *out_str << "020 [";

        if (field[0] & 0x80)
        {
          *out_str << "org=system coords ";
        } else
        {
          *out_str << "org=local coords ";
        }

        *out_str << "intensity=" << ((field[0] >> 4) & 7);
        *out_str << std::setprecision (2) << " heading=" << ((double)((field[0] >> 1) & 7) * (22.5));
        *out_str << "]";

        NEW_LINE (*out_str, 0, '\0');
      }

      weather.VectorSystemCoordinates = (field[0] & 0x80) ? true : false;
      weather.VectorIntensity = (field[0] >> 4) & 7;
      vector_orientation = (double)((field[0] >> 1) & 7) * (M_PI / 8);

      if (field[0] & 1)
      {
        cms_warning ("No extension defined so far");
      }

      field++;
    }
    // sequence of cartesian vectors in SPF notation (1 + 6 * n)
    if (*fspec & FSPEC04_I009_030)
    {
      unsigned        rep = field[0];
      field++;

      if (out_str)
      {
        *out_str << "030 [sequence of cartesian vectors rep=" << rep << "]";
      }

      for (unsigned i = 0; i < rep; i++)
      {
        if (scaling_factor_cartesian != 0)
        {
          RDFWeatherCartesianVector vector;
          vector.X = (double)(field2signed (field, 16)) * scaling_factor_cartesian;
          field += 2;
          vector.Y = (double)(field2signed (field, 16)) * scaling_factor_cartesian;
          field += 2;
          vector.Length = (double)(field2signed (field, 16)) * scaling_factor_cartesian;
          field += 2;
          vector.Azimuth = vector_orientation;

          if (out_str)
          {
            NEW_LINE (*out_str, 4, ' ');
            vector.to_stringstream (*out_str, one_line);
          }

          weather.CartesianVector.push_back (vector);
        } else
        {
          if (out_str)
          {
            NEW_LINE (*out_str, 4, ' ');
            *out_str << "[scaling_factor_cartesian=0]";
            NEW_LINE (*out_str, 0, '\0');
          }
        }
      }

      if (out_str)
      {
        NEW_LINE (*out_str, 0, '\0');
      }
    }
    // sychronisation/control signal (1)
    if (*fspec & FSPEC05_I009_060)
    {
      if (out_str)
      {
        *out_str << "060 [step number=" << static_cast < int >(field[0] >> 2) << "]";
        NEW_LINE (*out_str, 0, '\0');
      }

      if (field[0] & 0x03)
      {
        cms_warning ("009/060[synchronisation/control] should be zero (0x%02x)", field[0]);
      }
      weather.StepSN = (field[0] >> 2) & 0x3F;
      field++;
    }
    // time of day (3)
    if (*fspec & FSPEC06_I009_070)
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
    // processing status (3)
    if (*fspec & FSPEC07_I009_080)
    {
      if (out_str)
      {
        scaling_factor = ((char)field[0]) >> 3;
        unsigned short  reduction_stage = field[0] & 7;
        unsigned short  processing_parameters = field2unsigned (field, 15);

        *out_str << "080 [f=" << scaling_factor << " R=" << reduction_stage << " Q=" << processing_parameters << "]";

        if (field[2] & 1)
        {
          NEW_LINE (*out_str, 4, ' ');

          *out_str << "[I009/080 octet no. 3: " << std::hex << static_cast < int >(field[2]) << "]" << std::endl;
        }

        NEW_LINE (*out_str, 0, '\0');
      }

      scaling_factor = ((char)field[0]) >> 3;
      scaling_factor_map[key_scaling] = scaling_factor;
      scaling_factor_cartesian = exp2 (scaling_factor - 6) * 1852;
      //uint8_t reduction_stage = field[0] & 7;
      //unsigned short processing_parameters = bytes2 (field[2], field[1]) >> 1;
      if (field[2] & 1)
      {
        field++;
      }
      field += 3;
    }

    if (!(*fspec & FSPEC_FX))
    {
      goto end;
    }

    fspec++;

    // radar configuration and status (1+3n)
    if (*fspec & FSPEC08_I009_090)
    {
      uint8_t         rep = field[0];
      if (out_str)
      {
        *out_str << "090 [radar configuration and status rep=" << static_cast < int >(rep) << "]";

      }

      field++;

      for (unsigned i = 0; i < rep; i++)
      {
        if (out_str)
        {
          NEW_LINE (*out_str, 4, ' ');

          *out_str << std::fixed << std::setprecision (3)
            << "[sac=" << static_cast < int >(field[0]) << " sic=" << static_cast < int >(field[1]) << " reduction=" << static_cast < int >(field[2] & 7);

          if (field[2] & 0x10)
          {
            *out_str << " CP ";
          } else
          {
            *out_str << " - ";
          }

          if (field[2] & 0x08)
          {
            *out_str << " WO ";
          } else
          {
            *out_str << " - ";
          }

          *out_str << "]";
        }
        // radar.sac
        // radar.sic
        // radar.status
        field += 3;
      }

      if (out_str)
      {
        NEW_LINE (*out_str, 0, '\0');
      }
    }
    // vector count (2)
    if (*fspec & FSPEC09_I009_100)
    {
      if (out_str)
      {
        *out_str << "100 [total number of items=" << ((field[0] << 8) | (field[1])) << "]";

        NEW_LINE (*out_str, 0, '\0');
      }
      weather.TotalNumberOfItems = field2unsigned (field, 16);
      field += 2;
    }

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

int WeatherMessage_to_Asterix009 (deque < RDFWeatherMessage > &weather_list, void *data, int maxsize, int scaling_factor, unsigned &WrittenNumberOfItems)
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
  msg[0] = 9;                   // kategorie
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
      *fspec |= FSPEC01_I009_010;
      field[0] = weather.SAC;
      field[1] = weather.SIC;
      field += 2;
    }

    // message type
    switch (weather.MessageType)
    {
      case WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_1:
        {
          if ((size + 6 + 3) < maxsize)
          {
            *fspec |= FSPEC02_I009_000;
            uint8_t *orig_field = field;
            field[0] = 2;
            field++;
            send_i009_020 (fspec, field, weather);
            unsigned size_avail = maxsize - fspec_size - field_size - size - 10;
            unsigned origWrittenNumberOfItems = WrittenNumberOfItems;
            is_act_message_vector_empty = send_i009_030 (fspec, field, weather, scaling_factor, size_avail, WrittenNumberOfItems);
            if (origWrittenNumberOfItems == WrittenNumberOfItems) { field = orig_field; }
          }
          break;
        }
      case WEATHER_MESSAGE_TYPE_INTERMEDIATE_UPDATE_STEP:
        {
          *fspec |= FSPEC02_I009_000;
          field[0] = 253;
          field++;
          send_i009_060 (fspec, field, weather);
          if (weather.Time.Present)
          {
            send_i009_070 (fspec, field, weather);
          }
          send_i009_080 (fspec, field, scaling_factor);

          field++;

          break;
        }
      case WEATHER_MESSAGE_TYPE_START_OF_PICTURE:
        {
          *fspec |= FSPEC02_I009_000;
          field[0] = 254;
          field++;

          send_i009_060 (fspec, field, weather);
          if (weather.Time.Present)
          {
            send_i009_070 (fspec, field, weather);
          }
          send_i009_080 (fspec, field, scaling_factor);

          *fspec |= FSPEC_FX;
          fspec++;

          send_i009_090 (fspec, field, weather);
          is_act_message_vector_empty = true;
          break;
        }
      case WEATHER_MESSAGE_TYPE_END_OF_PICTURE:
        {
          *fspec |= FSPEC02_I009_000;
          field[0] = 255;
          field++;

          send_i009_060 (fspec, field, weather);
          if (weather.Time.Present)
          {
            send_i009_070 (fspec, field, weather);
          }
          send_i009_080 (fspec, field, scaling_factor);

          *fspec |= FSPEC_FX;
          fspec++;

          send_i009_100 (fspec, field, weather);
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
        weather_list.pop_front (); // toto musime navzdy zahodit
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

  if (size <= 7) // CAT, LEN1, LEN2 [,FSPEC, SAC, SIC]
  {
    return 0;
  }

  msg[1] = (size >> 8) & 0xff;
  msg[2] = size & 0xff;
  return size;
}


void send_i009_020 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC03_I009_020;
  field[0] |= (weather.VectorSystemCoordinates ? 1 : 0) << 7; // only WEATHER_COORDINATES_SYSTEM
  field[0] |= (weather.VectorIntensity & 7) << 4;
  field[0] |= (4 << 1);         // pouze horizontalni vektory

  field++;
}


// Sequence of Cartesian vectors
bool send_i009_030 (uint8_t * fspec, uint8_t * &field, RDFWeatherMessage & weather, const int scaling_factor, unsigned maxsize, unsigned &WrittenNumberOfItems)
{
  unsigned        max_message_items = MIN(maxsize / 6, UINT8_MAX);  //cat(1),size(2),fpsec(1),010(2),000(1),020(1),030(1+6n)
  // max_message_items = max_message_items < 20 ? max_message_items : 20; // viz. popis i009/030, 2.2.1.4
  unsigned        rep = weather.CartesianVector.size ();
  uint8_t        *p_size;
  double          local_scaling_factor = exp2 (scaling_factor - 6);

  *fspec |= FSPEC04_I009_030;
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

    RDFWeatherCartesianVector vec = weather.CartesianVector.back ();
    weather.CartesianVector.pop_back ();
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
    if ((d_y_scaled < INT16_MIN) || (d_y_scaled > INT16_MAX)) usable = false;
    if (d_x_scaled > INT16_MAX) usable = false;
    if (usable)
    {
      if (d_x_scaled < INT16_MIN)
      {
        d_len_scaled += (d_x_scaled - INT16_MIN);
        d_x_scaled = INT16_MIN;
//        printf("[ CHANGE1] ");
      }
      if ((d_len_scaled + d_x_scaled) > INT16_MAX)
      {
        d_len_scaled = (INT16_MAX-1) - d_x_scaled;
//        printf("[ CHANGE2] ");
      }
      if (d_len_scaled <= 0.0) usable = false;
    }

    int16_t        x_scaled = (int16_t) d_x_scaled;
    int16_t        y_scaled = (int16_t) d_y_scaled;
    uint16_t       len_scaled = (uint16_t) d_len_scaled;

    if (len_scaled == 0) usable = false;
    if (usable)
    {
      field[0] = (x_scaled >> 8) & 0xff;
      field[1] = x_scaled & 0xff;
      field[2] = (y_scaled >> 8) & 0xff;
      field[3] = y_scaled & 0xff;
      field[4] = (len_scaled >> 8) & 0xff;
      field[5] = len_scaled & 0xff;
      field += 6;
      WrittenNumberOfItems++;
      (*p_size)++;
//      printf(" >> Y=%9.3f X=%9.3f LEN=%9.3f",d_y_scaled,d_x_scaled,d_len_scaled);
//      printf(" >> Y=%05d X=%05d LEN=%05d\n",y_scaled,x_scaled,len_scaled);
//    } else {
//      printf(" ... REMOVED\n");
    }
  }

  return true;
}




// Synchronisation/Control Signal
void send_i009_060 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC05_I009_060;
  field[0] = (weather.StepSN << 2) & 0xFC;
  field++;

  return;
}

// Time of Day
void send_i009_070 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC06_I009_070;

  int             time = RDF_time_to_asterix_time(weather.Time.Time);
  field[0] = (time >> 16) & 0xff;
  field[1] = (time >> 8) & 0xff;
  field[2] = time & 0xff;
  field += 3;

  return;
}

// Processing Status
void send_i009_080 (uint8_t * fspec, uint8_t * &field, const int scaling_factor)
{
  *fspec |= FSPEC07_I009_080;

  uint8_t         R = (0 & 0x03);       // reduction stage, normal operation = 0, Apps dependent
  field[0] = (scaling_factor & 0x1f) << 3;
  field[0] |= R;
  field[1] = 0;                 // Processing parameters. Apps dependent
  field[2] = 0;
  field += 3;

  return;
}

// Radar Config/Status
void send_i009_090 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC08_I009_090;
  field[0] = 1;
  field[1] = weather.SAC;
  field[2] = weather.SIC;
  field[3] = 0;
  field += 4;

  return;
}

// Vector Count
void send_i009_100 (uint8_t * fspec, uint8_t * &field, const RDFWeatherMessage & weather)
{
  *fspec |= FSPEC09_I009_100;

  field[0] = (weather.TotalNumberOfItems >> 8) & 0xff;
  field[1] = weather.TotalNumberOfItems & 0xff;
  field += 2;

  return;
}
