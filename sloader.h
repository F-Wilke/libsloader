typedef struct enablex_builtin {
    void * DynLoaderP;
    const char * name;
} ENABLEX_BUILTIN;

typedef struct data_section {

} DATA_SECTION;

int run_main(int argc, char* const argv[], char** envp);

Elf64_Half GetEType(const std::filesystem::path& filepath);

