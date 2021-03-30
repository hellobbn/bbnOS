//===- time.c - Time functions -------------------------------------------===//
//
// Implementations of time related functions.
//
//===---------------------------------------------------------------------===//
void delay(int time) {
  for (int i = 0; i < time; ++i) {
    for (int j = 0; j < 3000; ++j) {
      for (int k = 0; k < 100000; ++k) {
        // do nothing, about 1 second
      }
    }
  }
}
