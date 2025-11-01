#pragma once

#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "unit_of_work.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(UnitOfWorkFactory& unit_factory) : unit_factory_(unit_factory) {}

    void AddAuthor(const std::string& name) override;
    void AddAuthorWithId(const domain::AuthorId& id, const std::string& name) override;
    void DeleteAuthor(const domain::AuthorId& id) override;
    void EditAuthor(const domain::AuthorId& id, const std::string& new_name) override;

    domain::Authors GetAllAuthors() override;
    std::optional<domain::Author> FindAuthorById(const domain::AuthorId& id) override;
    std::optional<domain::Author> FindAuthorByName(const std::string& name) override;

    void AddBook(const domain::AuthorId& author_id, const std::string& title, int publication_year, domain::Tags tags,
                 const std::string& author_name) override;
    void DeleteBook(const domain::BookId& id) override;
    void EditBook(const domain::BookId& id, const std::string& title, int publication_year,
                  const domain::Tags& tags) override;

    domain::Books GetAllBooks() override;

    domain::Books GetBooksByAuthor(const domain::AuthorId& author_id) override;
    domain::Books GetBooksByTitle(const std::string& title) override;

private:
    UnitOfWorkFactory& unit_factory_;
};

}  // namespace app
