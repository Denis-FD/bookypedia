#pragma once

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;
using Tags = std::vector<std::string>;

class Book {
public:
    Book(BookId book_id, AuthorId author_id, std::string title, int publication_year, Tags tags)
        : book_id_(std::move(book_id)), author_id_(std::move(author_id)), title_(std::move(title)),
          publication_year_(publication_year), tags_(std::move(tags)) {}

    Book(BookId book_id, AuthorId author_id, std::string title, int publication_year, Tags tags,
         std::string author_name)
        : book_id_(std::move(book_id)), author_id_(std::move(author_id)), title_(std::move(title)),
          publication_year_(publication_year), tags_(std::move(tags)), author_name_(std::move(author_name)) {}

    const BookId& GetBookId() const noexcept {
        return book_id_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    const Tags& GetTags() const noexcept {
        return tags_;
    }

    int GetPublicationYear() const noexcept {
        return publication_year_;
    }

    const std::string& GetAuthorName() const noexcept {
        return author_name_;
    }

private:
    BookId book_id_;
    AuthorId author_id_;
    std::string author_name_{};
    std::string title_;
    int publication_year_ = 0;
    Tags tags_;
};

using Books = std::vector<Book>;

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual Books GetAllBooks() = 0;
    virtual Books GetBooksByAuthorId(const AuthorId& author_id) = 0;
    virtual Books GetBooksByTitle(const std::string& title) = 0;
    virtual void DeleteBookTags(const BookId& book_id) = 0;
    virtual void DeleteBook(const BookId& book_id) = 0;
    virtual void EditBook(const BookId& id, const std::string& title, int publication_year, const Tags& tags) = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
