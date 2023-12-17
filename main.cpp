#include "param_sfo.cpp"
#include <iostream>
#define OUT "f" //command to write output in file

int main(int argc, char* argv[]) {
 try {
  if(argc > 3) {
   throw std::invalid_argument("Invalid command");
  }
  std::string sfo_path = "", out_path = "";
  for(int i = 1; i < argc; i++) {
   std::string cmd(argv[i]);
   if(cmd == OUT) {
    out_path = "out.txt";
   }
   else {
    sfo_path = cmd;
   }
  }
  if(sfo_path == "") {
   throw std::invalid_argument("Invalid sfo file path");
  }
  param_sfo::param_sfo_file sfo(sfo_path);
  if(out_path != "") {
   std::ofstream out_stream(out_path, std::ofstream::out);
   if(!out_stream.is_open()) {
    throw std::invalid_argument("Unable to open output file");
   }
   out_stream<<"PATH"<<std::endl<<sfo_path<<std::endl<<std::endl;
   sfo.print(out_stream);
   out_stream.close();
  }
  else {
   std::cout<<"PATH"<<std::endl<<sfo_path<<std::endl<<std::endl;
   sfo.print(std::cout);
  }
 }
 catch(const std::exception& e) {
  std::cout<<e.what()<<std::endl;
 }
 return 0;
}