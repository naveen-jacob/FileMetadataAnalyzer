#include <iostream>
#include <magic.h>
#include <filesystem>
#include <chrono>


class MetadataAnalyzer {
    std::filesystem::path filePath;

    MetadataAnalyzer() {
        filePath = std::filesystem::current_path();
        filePath.append("Report.pdf");
    }

    void analyze() {
        std::filesystem::file_time_type last_write_time = std::filesystem::last_write_time(filePath);
        // const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(last_write_time);
        // const auto mtime = std::chrono::system_clock::to_time_t(systemTime);
        // std::cout << "File last write time: " << mtime << std::endl;

    }
};

int main()
{
    const char *actual_file = "Report.pdf";
    const char *magic_full;
    magic_t magic_cookie;

    /* MAGIC_MIME tells magic to return a mime of the file,
       but you can specify different things	*/
    magic_cookie = magic_open(MAGIC_MIME);

    if (magic_cookie == NULL) {
        printf("unable to initialize magic library\n");
        return 1;
    }

    if (magic_load(magic_cookie, NULL) != 0) {
        printf("cannot load magic database - %s\n", magic_error(magic_cookie));
        magic_close(magic_cookie);
        return 1;
    }

    magic_full = magic_file(magic_cookie, actual_file);
    std::cout << magic_full << std::endl;
    magic_close(magic_cookie);
    return 0;
}