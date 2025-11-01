#include "view.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <iostream>
#include <unordered_set>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    out << book.title << " by "sv << book.author_name << ", "sv << book.publication_year;
    return out;
}

void NormalizeTag(std::string& tag) {
    tag = boost::regex_replace(tag, boost::regex("\\s+"), " ");  // " +"
    boost::algorithm::trim(tag);
}

std::vector<std::string> PrepareTags(std::vector<std::string> raw_tags) {
    std::unordered_set<std::string> tags_set;
    std::vector<std::string> result;
    result.reserve(raw_tags.size());

    for (auto& tag : raw_tags) {
        NormalizeTag(tag);
        if (!tag.empty() && tags_set.insert(tag).second) {
            result.emplace_back(std::move(tag));
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

std::string NormalizeInput(std::istream& input) {
    std::string line;
    std::getline(input, line);
    boost::algorithm::trim(line);
    return line;
}

std::string FormatTags(const std::vector<std::string>& tags) {
    return boost::algorithm::join(tags, ", ");
}

void PrintBook(std::ostream& out, const detail::BookInfo& book) {
    out << "Title: "sv << book.title << std::endl;
    out << "Author: "sv << book.author_name << std::endl;
    out << "Publication year: "sv << book.publication_year << std::endl;
    if (book.tags.empty()) {
        return;
    }
    out << "Tags: "sv << FormatTags(book.tags) << std::endl;
}

}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " "sv << value << std::endl;
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}, use_cases_{use_cases}, input_{input}, output_{output} {
    menu_.AddAction("AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1));
    // или [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    menu_.AddAction("DeleteAuthor"s, "name"s, "Deletes author"s, std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, "name"s, "Edits author"s, std::bind(&View::EditAuthor, this, ph::_1));
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s, std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, "<title>"s, "Deletes book"s, std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, "<title>"s, "Edits book"s, std::bind(&View::EditBook, this, ph::_1));
    menu_.AddAction("ShowBook"s, "<title>"s, "Show book"s, std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s, std::bind(&View::ShowAuthorBooks, this));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name = detail::NormalizeInput(cmd_input);

        if (name.empty()) {
            throw std::runtime_error("Empty author name"s);
        }

        if (auto author = use_cases_.FindAuthorByName(name)) {
            throw std::runtime_error("This author already exists"s);
        }

        use_cases_.AddAuthor(std::move(name));

    } catch (const std::exception& ex) {
        output_ << "Failed to add author: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    try {
        std::string name = detail::NormalizeInput(cmd_input);
        auto author = FindAuthorByNameOrSelect(name);

        if (!author) {
            throw std::runtime_error("Author not found or not selected"s);
        }

        use_cases_.DeleteAuthor(domain::AuthorId::FromString(author->id));

    } catch (const std::exception& ex) {
        output_ << "Failed to delete author: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::EditAuthor(std::istream& cmd_input) const {
    try {
        std::string name = detail::NormalizeInput(cmd_input);
        auto author = FindAuthorByNameOrSelect(name);

        if (!author) {
            throw std::runtime_error("Author not found or not selected"s);
        }

        output_ << "Enter new name:"sv << std::endl;
        std::string new_name = detail::NormalizeInput(input_);

        if (new_name.empty()) {
            throw std::runtime_error("Empty input new name"s);
        }

        use_cases_.EditAuthor(domain::AuthorId::FromString(author->id), new_name);

    } catch (const std::exception& ex) {
        output_ << "Failed to edit author: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        auto params = GetBookParams(cmd_input);

        if (!params) {
            throw std::runtime_error("Invalid book parameters"s);
        }

        use_cases_.AddBook(domain::AuthorId::FromString(std::move(params->author_id)), std::move(params->title),
                           params->publication_year, std::move(params->tags), std::move(params->author_name));

    } catch (const std::exception& ex) {
        output_ << "Failed to add book: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::DeleteBook(std::istream& cmd_input) const {
    try {
        auto book = SelectBookByTitle(cmd_input);

        if (!book) {
            return true;
        }

        use_cases_.DeleteBook(domain::BookId::FromString(book->id));

    } catch (const std::exception& ex) {
        output_ << "Failed to delete book: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::EditBook(std::istream& cmd_input) const {
    try {
        auto book = SelectBookByTitle(cmd_input);

        if (!book) {
            output_ << "Book not found"sv << std::endl;
            return true;
        }

        book->title = ReadNewTitle(book->title);
        book->publication_year = ReadNewYear(book->publication_year);
        book->tags = ReadNewTags(book->tags);

        use_cases_.EditBook(domain::BookId::FromString(book->id), book->title, book->publication_year, book->tags);

    } catch (const std::exception& ex) {
        output_ << "Failed to edit book: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::ShowBook(std::istream& cmd_input) const {
    try {
        auto selected_book = SelectBookByTitle(cmd_input);

        if (!selected_book) {
            return true;
        }

        detail::PrintBook(output_, *selected_book);

    } catch (const std::exception& ex) {
        output_ << "Failed to Show Book: "sv << ex.what() << std::endl;
    }
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetAllBooks());
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::ShowAuthorBooks() const {
    try {
        auto author_id = SelectAuthor();

        if (!author_id) {
            throw std::runtime_error("Author not found"s);
        }

        PrintVector(output_, GetAuthorBooks(*author_id));
    } catch (const std::exception& ex) {
        output_ << "Failed to Show Books: "sv << ex.what() << std::endl;
    }
    return true;
}

// --- --- --- --- --- --- --- --- --- ---

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    if (!(cmd_input >> params.publication_year)) {
        return std::nullopt;
    }

    params.title = detail::NormalizeInput(cmd_input);

    if (params.title.empty()) {
        return std::nullopt;
    }

    auto author_info = SelectAuthorOrAddNew();
    if (!author_info) {
        return std::nullopt;
    }

    params.author_id = author_info->id;
    params.author_name = author_info->name;

    output_ << "Enter tags (comma separated):"sv << std::endl;
    params.tags = GetBookTags();

    return params;
}

std::optional<detail::AuthorInfo> View::SelectAuthorOrAddNew() const {
    output_ << "Enter author name or empty line to select from list:"sv << std::endl;

    std::string name = detail::NormalizeInput(input_);

    if (auto author = FindAuthorByNameOrSelect(name)) {
        return *author;
    }

    output_ << "No author found. Do you want to add "sv << name << " (y/n)?"sv << std::endl;
    std::string answer = detail::NormalizeInput(input_);
    if (answer != "y" && answer != "Y") {
        return std::nullopt;
    }

    auto id = domain::AuthorId::New();
    use_cases_.AddAuthorWithId(id, name);
    return detail::AuthorInfo{id.ToString(), name};
}

std::optional<detail::AuthorInfo> View::FindAuthorByNameOrSelect(const std::string& name) const {
    if (name.empty()) {
        return SelectAuthor();
    }

    if (auto author = use_cases_.FindAuthorByName(name)) {
        return detail::AuthorInfo{author->GetId().ToString(), author->GetName()};
    }

    return std::nullopt;
}

std::optional<detail::AuthorInfo> View::SelectAuthor() const {
    output_ << "Select author:"sv << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel"sv << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num"s);
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num"s);
    }

    return authors[author_idx];
}

std::optional<detail::BookInfo> View::SelectBook(std::vector<detail::BookInfo> books) const {
    if (books.empty()) {
        return std::nullopt;
    }

    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel:"sv << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    size_t book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid book num"s);
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num"s);
    }

    return books[book_idx];
}

std::vector<std::string> View::GetBookTags() const {
    std::string input_tags;
    if (!std::getline(input_, input_tags) || input_tags.empty()) {
        return {};
    }

    std::vector<std::string> raw_tags;
    boost::split(raw_tags, input_tags, boost::is_any_of(","));

    return detail::PrepareTags(std::move(raw_tags));
}

std::optional<detail::BookInfo> View::SelectBookByTitle(std::istream& cmd_input) const {
    std::string title = detail::NormalizeInput(cmd_input);

    if (title.empty()) {
        return SelectBook(GetAllBooks());
    }

    auto same_title_books = GetBooksByTitle(title);

    if (same_title_books.empty()) {
        return std::nullopt;
    }

    if (same_title_books.size() == 1) {
        return same_title_books.front();
    }

    return SelectBook(same_title_books);
}

std::string View::ReadNewTitle(const std::string& current_title) const {
    output_ << "Enter new title or empty line to use the current one ("sv << current_title << "):"sv << std::endl;
    std::string title = detail::NormalizeInput(input_);
    return title.empty() ? current_title : title;
}

int View::ReadNewYear(int current_year) const {
    output_ << "Enter publication year or empty line to use the current one ("sv << current_year << "):"sv << std::endl;
    std::string year_str = detail::NormalizeInput(input_);
    if (year_str.empty()) {
        return current_year;
    }
    try {
        return std::stoi(year_str);
    } catch (...) {
        throw std::runtime_error("Invalid publication year"s);
    }
}

std::vector<std::string> View::ReadNewTags(const std::vector<std::string>& current_tags) const {
    output_ << "Enter tags (current tags: "sv << detail::FormatTags(current_tags) << "):"sv << std::endl;
    return GetBookTags();
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    const auto& all_authors = use_cases_.GetAllAuthors();
    std::vector<detail::AuthorInfo> result;
    result.reserve(all_authors.size());

    for (const auto& author : all_authors) {
        result.emplace_back(author.GetId().ToString(), author.GetName());
    }

    return result;
}

std::vector<detail::BookInfo> View::BooksToInfo(const domain::Books& books) const {
    std::vector<detail::BookInfo> result;
    result.reserve(books.size());

    for (const auto& book : books) {
        result.emplace_back(book.GetBookId().ToString(), book.GetTitle(), book.GetAuthorName(),
                            book.GetPublicationYear(), book.GetTags());
    }

    std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) {
        return std::tie(lhs.title, lhs.author_name, lhs.publication_year) <
               std::tie(rhs.title, rhs.author_name, rhs.publication_year);
    });

    return result;
}

std::vector<detail::BookInfo> View::GetAllBooks() const {
    return BooksToInfo(use_cases_.GetAllBooks());
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const detail::AuthorInfo& author) const {
    return BooksToInfo(use_cases_.GetBooksByAuthor(domain::AuthorId::FromString(author.id)));
}

std::vector<detail::BookInfo> View::GetBooksByTitle(const std::string& title) const {
    return BooksToInfo(use_cases_.GetBooksByTitle(title));
}

}  // namespace ui
