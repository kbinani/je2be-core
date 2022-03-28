/* This file was derived from libxna by MrMetric
 */
/** This file uses code from LzxDecoder.cs in MonoGame. The following is from the beginning of the file: **/
/* This file was derived from libmspack
 * (C) 2003-2004 Stuart Caie.
 * (C) 2011 Ali Scissons.
 *
 * The LZX method was created by Jonathan Forbes and Tomi Poutanen, adapted
 * by Microsoft Corporation.
 *
 * This source file is Dual licensed; meaning the end-user of this source file
 * may redistribute/modify it under the LGPL 2.1 or MS-PL licenses.
 */
/* GNU LESSER GENERAL PUBLIC LICENSE version 2.1
 * LzxDecoder is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 */
/*
 * MICROSOFT PUBLIC LICENSE
 * This source code is subject to the terms of the Microsoft Public License (Ms-PL).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * is permitted provided that redistributions of the source code retain the above
 * copyright notices and this file header.
 *
 * Additional copyright notices should be appended to the list above.
 *
 * For details, see <http://www.opensource.org/licenses/ms-pl.html>.
 */
/*
 * This derived work is recognized by Stuart Caie and is authorized to adapt
 * any changes made to lzxd.c in his libmspack library and will still retain
 * this dual licensing scheme. Big thanks to Stuart Caie!
 *
 * DETAILS
 * This file is a pure C# port of the lzxd.c file from libmspack, with minor
 * changes towards the decompression of XNB files. The original decompression
 * software of LZX encoded data was written by Suart Caie in his
 * libmspack/cabextract projects, which can be located at
 * http://http://www.cabextract.org.uk/
 */
#pragma once

namespace je2be::box360 {

class LzxDecoder {
public:
  explicit LzxDecoder(unsigned short window_bits) {
    // LZX supports window sizes of 2^15 (32 KiB) to 2^21 (2 MiB)
    if (window_bits < 15 || window_bits > 21) {
      throw std::runtime_error("LzxDecoder: unsupported window size exponent: " + std::to_string(window_bits));
    }

    window_size = 1 << window_bits;

    // let's initialize our state
    window = new unsigned char[window_size];
    memset(window, 0xDC, window_size);
    window_posn = 0;

    //// initialize tables
    // for (unsigned long i = 0, j = 0; i <= 50; i += 2)
    //{
    //	extra_bits[i] = extra_bits[i + 1] = static_cast<unsigned char>(j);
    //	if ((i != 0) && (j < 17))
    //	{
    //		++j;
    //	}
    // }
    // for (unsigned long i = 0, j = 0; i <= 50; ++i)
    //{
    //	position_base[i] = static_cast<unsigned long>(j);
    //	j += 1 << extra_bits[i];
    // }

    unsigned long posn_slots;
    if (window_bits == 20) {
      posn_slots = 42;
    } else if (window_bits == 21) {
      // note for future me: this 50 is likely related to the 50*8 in the MAINTREE_MAXSYMBOLS definition (see posn_slots * 8 below)
      posn_slots = 50;
    } else {
      posn_slots = window_bits * 2;
    }

    // reset state
    R0 = R1 = R2 = 1;
    main_elements = static_cast<unsigned short>(k_num_chars + (posn_slots * 8));
    header_read = false;
    block_remaining = 0;
    block_type = e_block_type::_lxz_block_type_invalid;

    // initialize tables to 0 (because deltas will be applied to them)
    memset(MAINTREE_len, 0, sizeof(MAINTREE_len));
    memset(LENGTH_len, 0, sizeof(LENGTH_len));
  }

  LzxDecoder(const LzxDecoder &) = delete;
  LzxDecoder &operator=(const LzxDecoder &) = delete;

  ~LzxDecoder() {
    delete[] window;
  }

  void decompress(
      const unsigned char *const compressed_buffer,
      unsigned long const compressed_buffer_length,
      unsigned char *uncompressed_buffer,
      const unsigned long uncompressed_buffer_length) {
    init_bits(compressed_buffer);

    static unsigned char const extra_bits[] = {
        0,
        0,
        0,
        0,
        1,
        1,
        2,
        2,
        3,
        3,
        4,
        4,
        5,
        5,
        6,
        6,
        7,
        7,
        8,
        8,
        9,
        9,
        10,
        10,
        11,
        11,
        12,
        12,
        13,
        13,
        14,
        14,
        15,
        15,
        16,
        16,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
        17,
    };

    static long const position_base_minus2[sizeof(extra_bits) / sizeof(extra_bits[0])] = {
        -2,
        -1,
        0,
        1,
        2,
        4,
        6,
        10,
        14,
        22,
        30,
        46,
        62,
        94,
        126,
        190,
        254,
        382,
        510,
        766,
        1022,
        1534,
        2046,
        3070,
        4094,
        6142,
        8190,
        12286,
        16382,
        24574,
        32766,
        49150,
        65534,
        98302,
        131070,
        196606,
        262142,
        393214,
        524286,
        655358,
        786430,
        917502,
        1048574,
        1179646,
        1310718,
        1441790,
        1572862,
        1703934,
        1835006,
        1966078,
        2097150,
        2228222,
        2359294,
        2490366,
        2621438,
        2752510,
        2883582,
        3014654,
        3145726,
        3276798,
        3407870,
        3538942,
        3670014,
        3801086,
        3932158,
        4063230,
        4194302,
        4325374,
        4456446,
        4587518,
        4718590,
        4849662,
        4980734,
        5111806,
        5242878,
        5373950,
        5505022,
        5636094,
        5767166,
        5898238,
        6029310,
        6160382,
        6291454,
        6422526,
        6553598,
        6684670,
        6815742,
        6946814,
        7077886,
        7208958,
        7340030,
        7471102,
        7602174,
        7733246,
        7864318,
        7995390,
        8126462,
        8257534,
        8388606,
        8519678,
        8650750,
        8781822,
        8912894,
        9043966,
        9175038,
        9306110,
        9437182,
        9568254,
        9699326,
        9830398,
        9961470,
        10092542,
        10223614,
        10354686,
        10485758,
        10616830,
        10747902,
        10878974,
        11010046,
        11141118,
        11272190,
        11403262,
        11534334,
        11665406,
        11796478,
        11927550,
        12058622,
        12189694,
        12320766,
        12451838,
        12582910,
        12713982,
        12845054,
        12976126,
        13107198,
        13238270,
        13369342,
        13500414,
        13631486,
        13762558,
        13893630,
        14024702,
        14155774,
        14286846,
        14417918,
        14548990,
        14680062,
        14811134,
        14942206,
        15073278,
        15204350,
        15335422,
        15466494,
        15597566,
        15728638,
        15859710,
        15990782,
        16121854,
        16252926,
        16383998,
        16515070,
        16646142,
        16777214,
        16908286,
        17039358,
        17170430,
        17301502,
        17432574,
        17563646,
        17694718,
        17825790,
        17956862,
        18087934,
        18219006,
        18350078,
        18481150,
        18612222,
        18743294,
        18874366,
        19005438,
        19136510,
        19267582,
        19398654,
        19529726,
        19660798,
        19791870,
        19922942,
        20054014,
        20185086,
        20316158,
        20447230,
        20578302,
        20709374,
        20840446,
        20971518,
        21102590,
        21233662,
        21364734,
        21495806,
        21626878,
        21757950,
        21889022,
        22020094,
        22151166,
        22282238,
        22413310,
        22544382,
        22675454,
        22806526,
        22937598,
        23068670,
        23199742,
        23330814,
        23461886,
        23592958,
        23724030,
        23855102,
        23986174,
        24117246,
        24248318,
        24379390,
        24510462,
        24641534,
        24772606,
        24903678,
        25034750,
        25165822,
        25296894,
        25427966,
        25559038,
        25690110,
        25821182,
        25952254,
        26083326,
        26214398,
        26345470,
        26476542,
        26607614,
        26738686,
        26869758,
        27000830,
        27131902,
        27262974,
        27394046,
        27525118,
        27656190,
        27787262,
        27918334,
        28049406,
        28180478,
        28311550,
        28442622,
        28573694,
        28704766,
        28835838,
        28966910,
        29097982,
        29229054,
        29360126,
        29491198,
        29622270,
        29753342,
        29884414,
        30015486,
        30146558,
        30277630,
        30408702,
        30539774,
        30670846,
        30801918,
        30932990,
        31064062,
        31195134,
        31326206,
        31457278,
        31588350,
        31719422,
        31850494,
        31981566,
        32112638,
        32243710,
        32374782,
        32505854,
        32636926,
        32767998,
        32899070,
        33030142,
        33161214,
        33292286,
        33423358,
        33554430,
    };

    // read header if necessary
    if (!header_read) {
      const unsigned long intel = read_bits(1);
      if (intel != 0) {
        throw std::runtime_error("LzxDecoder::Decompress: Intel E8 not supported");
      }
      header_read = true;
    }

    // main decoding loop
    unsigned long togo = uncompressed_buffer_length;
    while (togo > 0) {
      // last block finished, new block expected
      if (block_remaining == 0) {
        block_type = static_cast<e_block_type>(read_bits(3));

        const unsigned long hi = read_bits(16);
        const unsigned long lo = read_bits(8);
        block_remaining = block_length = static_cast<unsigned long>((hi << 8) | lo);

        switch (block_type) {
        case e_block_type::_lxz_block_type_aligned: {
          for (unsigned long i = 0; i < 8; ++i) {
            ALIGNED_len[i] = static_cast<unsigned char>(read_bits(3));
          }
          make_decode_table(k_aligned_max_symbols, k_aligned_table_bits, ALIGNED_len, ALIGNED_table);
          // rest of aligned header is same as verbatim
#ifdef __clang__
          [[clang::fallthrough]];
#endif
        }

        case e_block_type::_lxz_block_type_verbatim: {
          read_lengths(MAINTREE_len, 0, 256);
          read_lengths(MAINTREE_len, 256, main_elements);
          make_decode_table(m_main_tree_max_symbols, k_main_tree_bits, MAINTREE_len, MAINTREE_table);

          read_lengths(LENGTH_len, 0, k_num_secondary_lengths);
          make_decode_table(k_length_max_symbols, k_length_table_bits, LENGTH_len, LENGTH_table);
          break;
        }

        case e_block_type::_lxz_block_type_uncompressed: {
          if (bit_buffer_bits_left == 0) {
            ensure_bits(16);
          }
          R0 = read_bits(32);
          R1 = read_bits(32);
          R2 = read_bits(32);
          break;
        }

        case e_block_type::_lxz_block_type_invalid:
        default: {
          throw std::runtime_error("LzxDecoder::Decompress: invalid state block type: " + std::to_string((unsigned char)block_type));
        }
        }
      }

      // buffer exhaustion check
      if (bit_buffer_input_position > compressed_buffer_length) {
        /*
                          it's possible to have a file where the next run is less than 16 bits in size. In this case, the READ_HUFFSYM() macro used in building
                          the tables will exhaust the buffer, so we should allow for this, but not allow those accidentally read bits to be used
                          (so we check that there are at least 16 bits remaining - in this boundary case they aren't really part of the compressed data)
                          */
        if (bit_buffer_input_position > (compressed_buffer_length + 2) || bit_buffer_bits_left < 16) {
          throw std::runtime_error("LzxDecoder::Decompress: invalid data");
        }
      }

      unsigned long this_run;
      while ((this_run = block_remaining) > 0 && togo > 0) {
        if (this_run > togo) {
          this_run = togo;
        }
        togo -= this_run;
        block_remaining -= this_run;

        // apply 2^x-1 mask
        window_posn &= window_size - 1;
        // runs can't straddle the window wraparound
        if ((window_posn + this_run) > window_size) {
          throw std::runtime_error("LzxDecoder::Decompress: invalid data (window position + this_run > window size)");
        }

        if (block_type == e_block_type::_lxz_block_type_verbatim || block_type == e_block_type::_lxz_block_type_aligned) {
          while (this_run > 0) {
            unsigned long main_element = read_huffman_symbols(MAINTREE_table, MAINTREE_len, m_main_tree_max_symbols, k_main_tree_bits);

            if (main_element < k_num_chars) {
              // literal: 0 to k_num_chars-1
              window[window_posn++] = static_cast<unsigned char>(main_element);
              --this_run;
            } else {
              // match: k_num_chars + ((slot<<3) | length_header (3 bits))
              main_element -= k_num_chars;

              unsigned long match_length = main_element & k_num_primary_lengths;
              if (match_length == k_num_primary_lengths) {
                unsigned long length_footer = read_huffman_symbols(LENGTH_table, LENGTH_len, k_length_max_symbols, k_length_table_bits);
                match_length += length_footer;
              }
              match_length += k_min_match;

              unsigned long match_offset = main_element >> 3;

              if (match_offset > 2) {
                // not repeated offset
                switch (block_type) {
                case e_block_type::_lxz_block_type_verbatim: {
                  {
                    if (match_offset != 3) {
                      unsigned char extra = extra_bits[match_offset];
                      unsigned long verbatim_bits = read_bits(extra);
                      // match_offset = position_base[match_offset] - 2 + verbatim_bits;
                      match_offset = position_base_minus2[match_offset] + verbatim_bits;
                    } else {
                      match_offset = 1;
                    }
                  }
                  break;
                }

                case e_block_type::_lxz_block_type_aligned: {
                  {
                    unsigned char extra = extra_bits[match_offset];
                    // match_offset = position_base[match_offset] - 2;
                    match_offset = position_base_minus2[match_offset];
                    if (extra > 3) {
                      // verbatim and aligned bits
                      extra -= 3;
                      unsigned long verbatim_bits = read_bits(extra);
                      match_offset += (verbatim_bits << 3);

                      unsigned long aligned_bits = read_huffman_symbols(ALIGNED_table, ALIGNED_len, k_aligned_max_symbols, k_aligned_table_bits);
                      match_offset += aligned_bits;
                    } else if (extra == 3) {
                      // aligned bits only
                      unsigned long aligned_bits = read_huffman_symbols(ALIGNED_table, ALIGNED_len, k_aligned_max_symbols, k_aligned_table_bits);
                      match_offset += aligned_bits;
                    } else if (extra > 0) // extra==1, extra==2
                    {
                      // verbatim bits only
                      unsigned long verbatim_bits = read_bits(extra);
                      match_offset += verbatim_bits;
                    } else // extra == 0
                    {
                      // ???
                      match_offset = 1;
                    }
                  }
                  break;
                }

                default:
                  break;
                }

                // update repeated offset LRU queue
                R2 = R1;
                R1 = R0;
                R0 = match_offset;
              } else if (match_offset == 0) {
                match_offset = R0;
              } else if (match_offset == 1) {
                match_offset = R1;
                R1 = R0;
                R0 = match_offset;
              } else // match_offset == 2
              {
                match_offset = R2;
                R2 = R0;
                R0 = match_offset;
              }

              unsigned long runsrc;
              unsigned long rundest = window_posn;

              if (match_length > this_run) {
                throw std::runtime_error("LzxDecoder::Decompress: match_length > this_run (" + std::to_string(match_length) + " > " + std::to_string(this_run) + ")");
              }
              this_run -= match_length;

              // copy any wrapped around source data
              if (window_posn >= match_offset) {
                // no wrap
                runsrc = rundest - match_offset;
              } else {
                runsrc = rundest + (window_size - match_offset);
                unsigned long copy_length = match_offset - window_posn;
                if (copy_length < match_length) {
                  match_length -= copy_length;
                  window_posn += copy_length;
                  copy_n_safe(window, copy_length, runsrc, rundest);
                  runsrc = 0;
                }
              }
              window_posn += match_length;

              // copy match data
              copy_n_safe(window, match_length, runsrc, rundest);
            }
          }
        } else if (block_type == e_block_type::_lxz_block_type_uncompressed) {
          if ((bit_buffer_input_position + this_run) > compressed_buffer_length) {
            throw std::runtime_error("LzxDecoder::Decompress: invalid data (inpos + this_run > endpos)");
          }

          memcpy(window + window_posn, compressed_buffer + bit_buffer_input_position, this_run);
          bit_buffer_input_position += this_run;
          window_posn += this_run;
        } else {
          throw std::runtime_error("LzxDecoder::Decompress: unknown block type");
        }
      }
    }
    if (togo != 0) {
      throw std::runtime_error("LzxDecoder::Decompress: togo != 0\n");
    }

    unsigned long start_window_pos = window_posn;
    if (start_window_pos == 0) {
      start_window_pos = window_size;
    }
    if (start_window_pos < uncompressed_buffer_length) {
      throw std::runtime_error("LzxDecoder::Decompress: invalid data (start_window_pos < outLen)");
    }
    start_window_pos -= uncompressed_buffer_length;
    memcpy(uncompressed_buffer, window + start_window_pos, uncompressed_buffer_length);
  }

  // Maybe equivalent to XMemDecompress
  static size_t Decode(std::vector<uint8_t> &buffer) {
    using namespace std;

    auto decoder = make_unique<LzxDecoder>(17);

    vector<uint8_t> out;
    size_t remaining = buffer.size();
    size_t pos = 0;
    size_t decodedBytes = 0;

    while (remaining > 0) {
      uint16_t inputSize = 0;
      uint16_t outputSize = 0;
      if (buffer[pos] == 0xff) {
        if (remaining < 5) {
          return 0;
        }
        outputSize = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + pos + 1));
        inputSize = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + pos + 3));

        remaining -= 5;
        pos += 5;
      } else {
        if (remaining < 2) {
          if (buffer[pos] == 0) {
            // EOS
            break;
          } else {
            // Unexpected. Recognize this situation as an error
            return 0;
          }
        }
        outputSize = 0x8000;
        inputSize = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + pos));
        remaining -= 2;
        pos += 2;
      }

      if (inputSize == 0) {
        break;
      } else if (inputSize > remaining) {
        return 0;
      }

      out.resize(out.size() + outputSize);
      try {
        decoder->decompress(buffer.data() + pos, inputSize, out.data() + decodedBytes, outputSize);
      } catch (...) {
        return 0;
      }

      pos += inputSize;
      decodedBytes += outputSize;
      remaining -= inputSize;
    }

    out.swap(buffer);

    return decodedBytes;
  }

private:
  static constexpr unsigned short k_min_match = 2;
  static constexpr unsigned short k_max_match = 257;
  static constexpr unsigned short k_num_chars = 256;
  static constexpr unsigned short k_pre_tree_num_elements = 20;
  static constexpr unsigned short k_aligned_num_elements = 8;
  static constexpr unsigned short k_num_primary_lengths = 7;
  static constexpr unsigned short k_num_secondary_lengths = 249;

  static constexpr unsigned short k_pre_tree_max_symbols = k_pre_tree_num_elements;
  static constexpr unsigned short m_main_tree_max_symbols = k_num_chars + 50 * 8;
  static constexpr unsigned short k_length_max_symbols = k_num_secondary_lengths + 1;
  static constexpr unsigned short k_aligned_max_symbols = k_aligned_num_elements;

  static constexpr unsigned char k_pre_tree_bits = 6;
  static constexpr unsigned char k_main_tree_bits = 12;
  static constexpr unsigned char k_length_table_bits = 12;
  static constexpr unsigned char k_aligned_table_bits = 7;

  static void copy_n_safe(unsigned char *buf, unsigned long len, unsigned long src, unsigned long &dest) {
    if (src == dest) {
      return;
    }

    unsigned char *bufsrc = buf + src;
    unsigned char *bufdest = buf + dest;
    if ((dest > src) && (src + len >= dest)) {
      unsigned long distance = dest - src;
      unsigned long copies = len / distance;
      unsigned long leftover = len % distance;
      for (unsigned long i = 0; i < copies; ++i) {
        memcpy(bufdest, bufsrc, distance);
        bufdest += distance;
      }
      memcpy(bufdest, bufsrc, leftover);
    } else // overlap does not matter
    {
      memcpy(bufdest, bufsrc, len);
    }
    dest += len;
  }

  void make_decode_table(unsigned short num_symbols, unsigned char num_bits, unsigned char *length, unsigned short *table) {
    unsigned long leaf;
    unsigned char bit_num = 1;
    unsigned long pos = 0; // the current position in the decode table
    // note: nbits is at most 12
    unsigned long table_mask = 1 << num_bits;

    // bit_mask never exceeds 15 bits
    unsigned short bit_mask = static_cast<unsigned short>(table_mask >> 1); // don't do 0 length codes

    unsigned short next_symbol = bit_mask; // base of allocation for long codes

    // fill entries for codes short enough for a direct mapping
    while (bit_num <= num_bits) {
      for (unsigned short sym = 0; sym < num_symbols; ++sym) {
        if (length[sym] == bit_num) {
          leaf = pos;

          if ((pos += bit_mask) > table_mask) {
            throw std::runtime_error("LzxDecoder::MakeDecodeTable: table overrun (1)");
          }

          // fill all possible lookups of this symbol with the symbol itself
          unsigned short *fill_start = table + leaf;
          unsigned short *fill_end = fill_start + bit_mask;
          while (fill_start < fill_end) {
            *fill_start = sym;
            fill_start++;
          }
        }
      }
      bit_mask >>= 1;
      ++bit_num;
    }

    // if there are any codes longer than nbits
    if (pos != table_mask) {
      // clear the remainder of the table
      memset(table + pos, 0, (table_mask - pos) * sizeof(*table));

      // give ourselves room for codes to grow by up to 16 more bits
      pos <<= 16;
      table_mask <<= 16;
      bit_mask = 1 << 15;

      while (bit_num <= 16) {
        for (unsigned short sym = 0; sym < num_symbols; ++sym) {
          if (length[sym] == bit_num) {
            leaf = pos >> 16;
            for (unsigned long fill = 0; fill < static_cast<unsigned long>(bit_num) - static_cast<unsigned long>(num_bits); ++fill) {
              // if this path hasn't been taken yet, 'allocate' two entries
              if (table[leaf] == 0) {
                table[(next_symbol << 1)] = 0;
                table[(next_symbol << 1) + 1] = 0;
                table[leaf] = (next_symbol++);
              }
              // follow the path and select either left or right for next bit
              leaf = static_cast<unsigned long>(table[leaf] << 1);
              if (((pos >> (15 - fill)) & 1) == 1) {
                ++leaf;
              }
            }
            table[leaf] = sym;

            if ((pos += bit_mask) > table_mask) {
              throw std::runtime_error("LzxDecoder::MakeDecodeTable: table overrun (2)");
            }
          }
        }
        bit_mask >>= 1;
        ++bit_num;
      }
    }

    // full table?
    if (pos == table_mask) {
      return;
    }

    // either erroneous table, or all elements are 0 - let's find out.
    for (unsigned short sym = 0; sym < num_symbols; ++sym) {
      if (length[sym] != 0) {
        throw std::runtime_error("LzxDecoder::MakeDecodeTable: erroneous table");
      }
    }
  }

  void read_lengths(unsigned char *lens, const unsigned long first, const unsigned long last) {
    // hufftbl pointer here?

    unsigned char pre_tree_lengths[k_pre_tree_max_symbols];

    for (unsigned long x = 0; x < k_pre_tree_max_symbols; ++x) {
      pre_tree_lengths[x] = static_cast<unsigned char>(read_bits(4));
    }
    make_decode_table(k_pre_tree_max_symbols, k_pre_tree_bits, pre_tree_lengths, PRETREE_table);

    for (unsigned long x = first; x < last;) {
      long z = read_huffman_symbols(PRETREE_table, pre_tree_lengths, k_pre_tree_max_symbols, k_pre_tree_bits);
      if (z == 17) {
        unsigned long y = read_bits(4);
        y += 4;
        memset(lens + x, 0, y);
        x += y;
      } else if (z == 18) {
        unsigned long y = read_bits(5);
        y += 20;
        memset(lens + x, 0, y);
        x += y;
      } else if (z == 19) {
        unsigned long y = read_bits(1);
        y += 4;
        z = read_huffman_symbols(PRETREE_table, pre_tree_lengths, k_pre_tree_max_symbols, k_pre_tree_bits);
        z = lens[x] - z;
        if (z < 0) {
          z += 17;
        }
        memset(lens + x, static_cast<unsigned char>(z), y);
        x += y;
      } else {
        z = lens[x] - z;
        if (z < 0) {
          z += 17;
        }
        lens[x++] = static_cast<unsigned char>(z);
      }
    }
  }

  unsigned long read_huffman_symbols(const unsigned short *table, const unsigned char *lengths, const unsigned long num_symbols, const unsigned char num_bits) {
    unsigned long i, j;
    ensure_bits(16);
    if ((i = table[peek_bits(num_bits)]) >= num_symbols) {
      j = static_cast<unsigned long>(1 << ((sizeof(unsigned long) * 8) - num_bits));
      do {
        j >>= 1;
        i <<= 1;
        i |= (bit_buffer_buffer & j) != 0 ? 1 : 0;
        if (j == 0) {
          throw std::runtime_error("LzxDecoder::ReadHuffSym: j == 0 in ReadHuffSym");
        }
      } while ((i = table[i]) >= num_symbols);
    }
    j = lengths[i];
    remove_bits(static_cast<unsigned char>(j));

    return i;
  }

  //static long const position_base_minus2[];
  //static unsigned char const extra_bits[];
  //unsigned long position_base[51];
  //unsigned char extra_bits[52];

  enum class e_block_type : unsigned char {
    _lxz_block_type_invalid,
    _lxz_block_type_verbatim,
    _lxz_block_type_aligned,
    _lxz_block_type_uncompressed,
  };

  unsigned char *window;
  unsigned long window_size;
  unsigned long window_posn;

  unsigned long R0;
  unsigned long R1;
  unsigned long R2;
  unsigned short main_elements;  // number of main tree elements
  bool header_read;              // have we started decoding at all yet?
  e_block_type block_type;       // type of this block
  unsigned long block_length;    // uncompressed length of this block
  unsigned long block_remaining; // uncompressed bytes still left to decode

  void init_bits(const unsigned char *input_buffer) {
    bit_buffer_buffer = 0;
    bit_buffer_bits_left = 0;
    bit_buffer_input_position = 0;
    bit_buffer_input_buffer = input_buffer;
  }

  void ensure_bits(const unsigned char bits) {
    while (bit_buffer_bits_left < bits) {
      unsigned short const read_bits = *reinterpret_cast<const unsigned short *>(bit_buffer_input_buffer + bit_buffer_input_position);
      bit_buffer_input_position += sizeof(unsigned short);

      unsigned char const amount_to_shift = sizeof(unsigned long) * 8 - 16 - bit_buffer_bits_left;
      bit_buffer_buffer |= static_cast<unsigned long>(read_bits) << amount_to_shift;
      bit_buffer_bits_left += 16;
    }
  }

  unsigned long peek_bits(const unsigned char bits) const {
    return (bit_buffer_buffer >> ((sizeof(unsigned long) * 8) - bits));
  }

  void remove_bits(const unsigned char bits) {
    bit_buffer_buffer <<= bits;
    bit_buffer_bits_left -= bits;
  }

  unsigned long read_bits(const unsigned char bits) {
    unsigned long ret = 0;

    if (bits > 0) {
      ensure_bits(bits);
      ret = peek_bits(bits);
      remove_bits(bits);
    }

    return ret;
  }

  unsigned long bit_buffer_buffer;
  unsigned char bit_buffer_bits_left;
  unsigned long bit_buffer_input_position;
  const unsigned char *bit_buffer_input_buffer;

  unsigned char MAINTREE_len[m_main_tree_max_symbols];
  unsigned char LENGTH_len[k_length_max_symbols];
  unsigned char ALIGNED_len[k_aligned_max_symbols];
  unsigned short PRETREE_table[(1 << k_pre_tree_bits) + (k_pre_tree_max_symbols * 2)];
  unsigned short MAINTREE_table[(1 << k_main_tree_bits) + (m_main_tree_max_symbols * 2)];
  unsigned short LENGTH_table[(1 << k_length_table_bits) + (k_length_max_symbols * 2)];
  unsigned short ALIGNED_table[(1 << k_aligned_table_bits) + (k_aligned_max_symbols * 2)];
};

} // namespace je2be::box360
