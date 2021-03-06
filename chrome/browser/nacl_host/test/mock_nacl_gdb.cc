// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <cstring>

#include "base/command_line.h"
#include "base/environment.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/process_util.h"

static const char kArgs[] = "--args";
static const char kEvalCommand[] = "--eval-command";
static const char kNaClIrt[] = "nacl-irt ";
static const char kPass[] = "PASS";

int main(int argc, char** argv) {
  scoped_ptr<base::Environment> env(base::Environment::Create());
  std::string mock_nacl_gdb_file;
  env->GetVar("MOCK_NACL_GDB", &mock_nacl_gdb_file);
  file_util::WriteFile(FilePath::FromUTF8Unsafe(mock_nacl_gdb_file),
                       kPass, strlen(kPass));
  CHECK_GE(argc, 3);
  // First argument should be --eval-command.
  CHECK_EQ(strcmp(argv[1], kEvalCommand), 0);
  // Second argument should start with nacl-irt.
  CHECK_GE(strlen(argv[2]), strlen(kNaClIrt));
  CHECK_EQ(strncmp(argv[2], kNaClIrt, strlen(kNaClIrt)), 0);
  char* irt_file_name = &argv[2][strlen(kNaClIrt)];
  FILE* irt_file = fopen(irt_file_name, "r");
  // nacl-irt parameter must be a file name.
  PCHECK(irt_file);
  fclose(irt_file);
  int i = 3;
  // Skip additional --eval-command parameters.
  while (i < argc) {
    if (strcmp(argv[i], kArgs) == 0) {
      i++;
      break;
    }
    if (strcmp(argv[i], kEvalCommand) == 0) {
      i += 2;
      // Command line shouldn't end with --eval-command switch without value.
      CHECK_LE(i, argc);
      continue;
    }
    // Unknown argument.
    NOTREACHED() << "Invalid argument " << argv[i];
  }
  // --args switch must be present.
  CHECK_LT(i, argc);

  CommandLine::StringVector arguments;
  for (; i < argc; i++) {
    arguments.push_back(
        CommandLine::StringType(argv[i], argv[i] + strlen(argv[i])));
  }
  CommandLine cmd_line(arguments);
  // Process must be launched successfully.
  PCHECK(base::LaunchProcess(cmd_line, base::LaunchOptions(), NULL));
  return 0;
}
