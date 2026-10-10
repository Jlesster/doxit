#include <sys/wait.h>
#include <unistd.h>
#include <trux/core.hpp>
#include <trux/input/key.hpp>
#include <trux/layout/layout.hpp>
#include <trux/renderer/renderer.hpp>
#include <trux/style/style.hpp>
#include <trux/terminal/terminal.hpp>
#include "docs.hpp"

using namespace trux;

enum class View { DIRECTORIES, DOC_FILES, W3M };

void run_w3m(Terminal& term, renderer::Renderer& renderer,
             const std::filesystem::path& file) {
    std::fflush(stdout);
    term.shutdown();  // hand the real terminal back

    pid_t pid = fork();
    if(pid == 0) {
        execlp("w3m", "w3m", file.c_str(), nullptr);
        _exit(127);  // exec failed
    }
    int status = 0;
    waitpid(pid, &status, 0);

    if(auto result = term.init(); !result) return;
    if(auto result = term.enable_raw_mode(); !result) return;
    renderer = renderer::Renderer(term.size());
}

int main() {
    auto term = Terminal();
    auto renderer = renderer::Renderer(term.size());
    auto folders = layout::init(term.size());
    auto files = layout::init(term.size());
    input::Input input;
    View view = View::DIRECTORIES;
    bool in_folder = false;

    Documents docs;

    if(auto result = term.init(); !result) return 1;
    if(auto result = term.enable_raw_mode(); !result) return 1;

    docs.doc_dirs = docs.parse_docs(DOC_PATH);
    int doc_dirs_selection = 0;
    int doc_dirs_offset = 0;
    int docs_selection = 0;
    int docs_offset = 0;

    auto documentation_dirs = component::Menu(docs.doc_dirs,
                                              doc_dirs_selection,
                                              doc_dirs_offset,
                                              &DocDirectory::title) |
                              component::BorderRounded;
    documentation_dirs.selected_style = style::Style::Reverse;
    auto documentation_files =
        component::Menu(docs.docs, docs_selection, docs_offset, &Docs::title) |
        component::BorderRounded;
    documentation_files.selected_style = style::Style::Reverse;

    while(!term.should_quit()) {
        renderer.resize(in_folder ? folders : files);
        renderer.begin_draw();
        {
            in_folder ? renderer.push(documentation_files, files)
                      : renderer.push(documentation_dirs, folders);
        }
        renderer.end_draw();
        term.present(renderer);

        if(auto event = input.poll(term)) {
            if(event.kind == input::EventKind::Quit) break;
            if(event.consumed && event.code != input::Key::Enter) continue;
            switch(view) {
                case View::DIRECTORIES: {
                    switch(event) {
                        case 'q': term.request_quit(); break;
                        case 'l':
                        case input::Key::Enter:
                            docs.docs = docs.iterate_directory(
                                docs.doc_dirs[doc_dirs_selection].path);
                            view = View::DOC_FILES;
                            in_folder = true;
                            break;
                        default: continue;
                    }
                } break;
                case View::DOC_FILES: {
                    switch(event) {
                        case 'q': term.request_quit(); break;
                        case 'h':
                        case input::Key::Escape:
                            view = View::DIRECTORIES;
                            in_folder = false;
                            break;
                        case input::Key::Enter:
                            run_w3m(
                                term, renderer, docs.docs[docs_selection].path);
                            break;
                        default: continue;
                    }
                } break;
                case View::W3M: {
                    switch(event) {
                        case 'h':
                        case input::Key::Escape:
                            view = View::DIRECTORIES;
                            in_folder = false;
                            break;
                        case input::Key::Enter:
                            run_w3m(
                                term, renderer, docs.docs[docs_selection].path);
                            view = View::W3M;
                            break;
                        default: continue;
                    }
                } break;
            }
        }
    }
    return 0;
}
