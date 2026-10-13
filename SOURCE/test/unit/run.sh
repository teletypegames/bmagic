#!/bin/sh
# Unit tests on the host: every layer above the platform (model, repositories on a fake
# disk, services, controllers, views, the whole program with scripted keys), with the
# host's C++ compiler (CXX), DOSBox not needed. ./run.sh [name] runs the tests whose name
# contains name.
set -e
cd "$(dirname "$0")/../.."
SOURCES="lib/core/*.cpp lib/ui/*.cpp lib/platform/file_system.cpp lib/platform/file_tree.cpp
         lib/platform/text_output.cpp lib/testing/scripted_keyboard.cpp lib/testing/screen_dump.cpp
         lib/testing/host/*.cpp lib/net/*.cpp lib/test/*.cpp
         model/*.cpp repository/*/*.cpp service/*.cpp controller/*.cpp view/*.cpp app/*.cpp test/unit/*.cpp"
python3 lib/tools/check_layers.py .
lib/tools/host_test.sh build/unit/tests $SOURCES
