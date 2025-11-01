#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include "../domain/book_fwd.h"

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    std::string author_name;
    int publication_year = 0;
    std::vector<std::string> tags;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string id;
    std::string title;
    std::string author_name;
    int publication_year = 0;
    std::vector<std::string> tags;
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;
    bool ShowBook(std::istream& cmd_input) const;
    bool ShowBooks() const;
    bool ShowAuthors() const;
    bool ShowAuthorBooks() const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::optional<detail::AuthorInfo> SelectAuthorOrAddNew() const;
    std::optional<detail::AuthorInfo> FindAuthorByNameOrSelect(const std::string& name) const;
    std::optional<detail::AuthorInfo> SelectAuthor() const;
    std::optional<detail::BookInfo> SelectBook(std::vector<detail::BookInfo> books) const;
    std::optional<detail::BookInfo> SelectBookByTitle(std::istream& cmd_input) const;
    std::vector<std::string> GetBookTags() const;
    std::vector<detail::AuthorInfo> GetAuthors() const;
    std::vector<detail::BookInfo> GetAllBooks() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const detail::AuthorInfo& author_id) const;
    std::vector<detail::BookInfo> GetBooksByTitle(const std::string& title) const;
    std::vector<detail::BookInfo> BooksToInfo(const domain::Books& books) const;

    std::string ReadNewTitle(const std::string& current_title) const;
    int ReadNewYear(int current_year) const;
    std::vector<std::string> ReadNewTags(const std::vector<std::string>& current_tags) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui
