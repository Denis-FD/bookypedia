#pragma once

#include <optional>
#include <string>

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual void AddAuthorWithId(const domain::AuthorId& id, const std::string& name) = 0;
    virtual void DeleteAuthor(const domain::AuthorId& id) = 0;
    virtual void EditAuthor(const domain::AuthorId& id, const std::string& new_name) = 0;

    virtual domain::Authors GetAllAuthors() = 0;
    virtual std::optional<domain::Author> FindAuthorById(const domain::AuthorId& id) = 0;
    virtual std::optional<domain::Author> FindAuthorByName(const std::string& name) = 0;

    virtual void AddBook(const domain::AuthorId& author_id, const std::string& title, int publication_year,
                         domain::Tags tags, const std::string& author_name) = 0;
    virtual void DeleteBook(const domain::BookId& id) = 0;
    virtual void EditBook(const domain::BookId& id, const std::string& title, int publication_year,
                          const domain::Tags& tags) = 0;

    virtual domain::Books GetAllBooks() = 0;

    virtual domain::Books GetBooksByAuthor(const domain::AuthorId& author_id) = 0;
    virtual domain::Books GetBooksByTitle(const std::string& title) = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
