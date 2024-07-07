#ifndef PARAM_SFO_H
#define PARAM_SFO_H
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

typedef uint8_t byte;
typedef uint16_t ushort;
typedef uint32_t uint;

//NOTE: all lengths are in bytes
//a PARAM.SFO file has a structure of key-datum correspondences: keys are entries of a keys_table and data are entries of a data_table
//a PARAM.SFO file contains an index_table where each entry connects a key to his corresponding datum, see index_table_entry

namespace param_sfo {

 enum fmt : ushort {
  utf8 = 0x0004, //utf-8 string
  utf8null = 0x0204, //utf-8 string null-terminated
  uint32 = 0x0404 //32-bit unsigned integer
 };

 //header starts at offset 0x00 and ends at offset 0x13, it has total length 0x14
 typedef struct {
  uint magic; //length 0x04
  uint version; //length 0x04
  uint keys_table_offset; //length 0x04, absolute start offset of keys_table
  uint data_table_offset; //length 0x04, absolute start offset of data_table
  uint tables_entries; //length 0x04, number of entries in keys_table, data_table, index_table
 } header_t;

 //index_table_entry has total length 0x10 and gives information about a keys_table_entry and corresponding data_table_entry, see param_entry_t
 typedef struct {
  ushort key_offset; //length 0x02, start offset of keys_table entry relative to keys_table_offset, absolute start offset is keys_table_offset + key_offset
  ushort datum_fmt; //length 0x02, type of data_table entry, see enum above
  uint datum_len; //length 0x04, length of data_table entry
  uint datum_max_len; //length 0x04, max length of data_table entry
  uint datum_offset; //length 0x04, start offset of data_table entry relative to data_table_offset, absolute start offset is data_table_offset + datum_offset
 } index_table_entry_t;

 //index_table starts at offset 0x14 and ends at offset tables_entries * 0x10 + 0x13
 typedef struct {
  std::vector<index_table_entry_t> entries;
 } index_table_t;

 typedef struct {
  std::string keys_table_entry;
  uint key_len;
  std::string data_table_entry;
  uint datum_len;
  std::string datum_fmt;
 } param_entry_t;

 //param_table starts at offset tables_entries * 0x10 + 0x14
 typedef struct {
  std::vector<param_entry_t> entries;
 } param_table_t;

 class param_sfo_file {
  private:
   const uint MAGIC = 0x46535000;
   void read_header(std::ifstream& in_stream);
   void read_index_table_entry(std::ifstream& in_stream, uint i, index_table_entry_t& index_table_entry);
   void read_index_table(std::ifstream& in_stream);
   void read_param_entry(std::ifstream& in_stream, index_table_entry_t& index_table_entry, param_entry_t& param_entry);
   void read_param_table(std::ifstream& in_stream);
   uint to_uint(byte LE_array[], int size);

  public:
   std::string path;
   header_t header;
   index_table_t index_table;
   param_table_t param_table;
   param_sfo_file(std::string& path);
   void print(std::ostream& out_stream, bool hex_format);
 };

}
#endif