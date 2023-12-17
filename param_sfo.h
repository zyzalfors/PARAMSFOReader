#ifndef PARAM_SFO_H
#define PARAM_SFO_H
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>
#define MAGIC 0x46535000

typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;

//NOTE: all lengths are in bytes
//param_sfo file has a structure of key-value correspondences, keys are entries of a keys_table and values (data) are entries of a data_table
//param_sfo file contains an index_table where each entry (index_table_entry) connects a key to his corresponding datum (value)

namespace param_sfo {

 enum fmt : ushort {
  utf8notnull = 0x0004, //utf-8 string not null-terminated
  utf8null = 0x0204, //utf-8 string null-terminated
  uint32 = 0x0404 //32-bit unsigned integer
 };

 //header starts at offset 0x00 and ends at offset 0x13, it has total length 0x14
 struct header {
  uint magic; //length 0x04
  uint version; //length 0x04
  uint keys_table_offset; //length 0x04, absolute start offset of keys_table
  uint data_table_offset; //length 0x04, absolute start offset of data_table
  uint tables_entries; //length 0x04, number of entries in keys_table, data_table and index_table
 };

 //index_table_entry is an entry of index_table, it has total length 0x10
 //index_table_entry gives information about a keys_table entry and corresponding data_table entry
 struct index_table_entry {
  ushort key_offset; //length 0x02, start offset of keys_table entry relative to keys_table_offset (absolute start offset is keys_table_offset + key_offset)
  ushort datum_fmt; //length 0x02, type of data_table entry, see enum above
  uint datum_len; //length 0x04, length of data_table entry
  uint datum_max_len; //length 0x04, max length of data_table entry
  uint datum_offset; //length 0x04, start offset of data_table entry relative to data_table_offset (absolute start offset is data_table_offset + datum_offset)
 };

 //index_table starts at offset 0x14 and ends at offset tables_entries x 0x10 + 0x13
 //index_table is constituted by a list of index_table_entry items
 struct index_table {
  std::vector<struct index_table_entry> table_entries;
 };

 class param_sfo_file {
  private:
   std::string filepath;
   struct header head;
   struct index_table table;
   void read_header(std::ifstream& stream);
   void read_index_table_entry(std::ifstream& stream, struct index_table_entry& table_entry, uint i);
   void read_index_table(std::ifstream& stream);
   uint convert_little_endian_array_to_uint(byte array[], int size);
 
  public:
   param_sfo_file(std::string _filepath);
   void print(std::ostream& out_stream);
 };

}
#endif
