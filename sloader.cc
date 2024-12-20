#include <elf.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "dyn_loader.h"
#include "exec_loader.h"
#include "utils.h"
#include "sloader.h"

void write_sloader_dummy_to_secure_tls_space();

Elf64_Half GetEType(const std::filesystem::path& filepath) {
    int fd = open(filepath.c_str(), O_RDONLY);
    CHECK(fd >= 0);

    size_t size = lseek(fd, 0, SEEK_END);
    CHECK_GT(size, 8UL + 16UL);

    size_t mapped_size = (size + 0xfff) & ~0xfff;

    char* p = (char*)mmap(NULL, mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
    LOG(FATAL) << LOG_BITS(mapped_size) << LOG_BITS(size) << LOG_KEY(filepath);
    CHECK(p != MAP_FAILED);

    Elf64_Ehdr* ehdr = reinterpret_cast<Elf64_Ehdr*>(p);
    return ehdr->e_type;
}

int run_main(int argc, char* const argv[], char** envp) {
    std::string argv0 = argv[1];
    std::filesystem::path fullpath;

    for (int i = 0; i < argc; i++) {
      printf("argv[%d] = %s\n", i, argv[i]);
    }

    if (argv0[0] == '.' || argv0.find("/") != std::string::npos) {
        fullpath = std::filesystem::path(argv0);
    } else {
        std::vector<std::string> path_dirs = SplitWith(std::string(getenv("PATH")), ":");
        for (const auto& dir : path_dirs) {
            std::filesystem::path p = std::filesystem::path(dir) / argv0;
            if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
                fullpath = p;
                break;
            }
        }
    }

    LOG(INFO) << LOG_KEY(fullpath);
    CHECK(std::filesystem::exists(fullpath));

    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.emplace_back(argv[i]);
    }

    std::vector<std::string> envs;
    for (char** env = envp; *env != 0; env++) {
        envs.emplace_back(*env);
    }

    Elf64_Half etype = GetEType(fullpath);
    if (etype == ET_DYN || etype == ET_EXEC) {
        InitializeDynLoader(fullpath, envs, args);
        
        GetDynLoader()->Run();
    } else {
        LOG(FATAL) << "Unsupported etype = " << LOG_KEY(etype);
        std::abort();
    }

    return 0;
}






int main(int argc, char* const argv[], char** envp) {
    return run_main(argc, argv, envp);
}
