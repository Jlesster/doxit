#include "docs.hpp"
#include <filesystem>

std::vector<DocDirectory> Documents::parse_docs(
    const std::filesystem::path& p) {
    std::vector<DocDirectory> doc_dirs;
    for(auto& it : std::filesystem::directory_iterator(p)) {
        DocDirectory doc;
        doc.path = it.path();
        doc.title = it.path().filename();
        doc_dirs.push_back(doc);
    }
    return doc_dirs;
}

std::vector<Docs> Documents::iterate_directory(const std::filesystem::path& p) {
    std::vector<Docs> docs;
    for(auto& it : std::filesystem::directory_iterator(p)) {
        Docs doc;
        doc.is_directory = it.is_directory();
        doc.path = it.path();
        doc.title = it.path().filename();
        docs.push_back(doc);
    }
    return docs;
}
