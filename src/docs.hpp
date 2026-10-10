#pragma once

#include <filesystem>
#include <string>
#include <vector>

const std::filesystem::path DOC_PATH = "/usr/share/doc";

struct DocDirectory {
    std::string title;
    std::filesystem::path path;
};
struct Docs {
    bool is_directory;
    std::string title;
    std::filesystem::path path;
};

struct Documents {
public:
    std::vector<DocDirectory> parse_docs(const std::filesystem::path& p);
    std::vector<Docs> iterate_directory(const std::filesystem::path& p);

    std::vector<DocDirectory> doc_dirs;
    std::vector<Docs> docs;
};
