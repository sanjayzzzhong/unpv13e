// Copyright 2020 Sanjay Zhong
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
  char buf[1024] = {0};
  snprintf(buf, sizeof("hello"), "hello");
  cout << strlen(buf) << endl;
  return 0;
}
