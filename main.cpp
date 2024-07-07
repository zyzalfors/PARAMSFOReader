#include "param_sfo.cpp"
#include <iostream>
#define OUT "/f" //command to print output data in file
#define HEX "/x" //command to print output data in hex format

int main(int argc, char* argv[]) {
 try {
  if(argc > 4) throw std::invalid_argument("Invalid command");
  std::string in_path, out_path;
  bool hex_format = false;
  for(int i = 1; i < argc; i++) {
   std::string cmd(argv[i]);
   if(cmd == OUT) out_path = "out.txt";
   else if(cmd == HEX) hex_format = true;
   else in_path = cmd;
  }
  if(in_path.size() == 0) throw std::invalid_argument("Invalid PARAM.SFO file path");
  param_sfo::param_sfo_file sfo_file(in_path);
  if(out_path.size() > 0) {
   std::ofstream out_stream(out_path, std::ofstream::out);
   if(!out_stream.is_open()) throw std::invalid_argument("Unable to open output file");
   sfo_file.print(out_stream, hex_format);
   out_stream.close();
  }
  else sfo_file.print(std::cout, hex_format);
 }
 catch(const std::exception& e) {
  std::cout << e.what() << std::endl;
 }
 return 0;
}
