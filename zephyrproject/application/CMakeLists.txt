cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(my_app)
zephyr_include_directories(${ZEPHYR_BASE}/../modules/lib/gui/lvgl)
target_sources(app PRIVATE
  src/main.c
  src/gui.c
  src/worker.c
  src/utils/monitor.c
)

target_include_directories(app PRIVATE include)