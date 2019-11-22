#include "app.h"

int main() {
  std::unique_ptr<frame::App> app(new frame::App());
  app->init();
  app->start();

  return 0;
}
