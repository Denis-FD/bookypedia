#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    auto uow = unit_factory_.GetUnitOfWork();
    uow->Authors().Save({AuthorId::New(), name});
    uow->Commit();
}

void UseCasesImpl::AddAuthorWithId(const domain::AuthorId& id, const std::string& name) {
    auto uow = unit_factory_.GetUnitOfWork();
    uow->Authors().Save({id, name});
    uow->Commit();
}

void UseCasesImpl::DeleteAuthor(const domain::AuthorId& id) {
    auto uow = unit_factory_.GetUnitOfWork();
    auto books = uow->Books().GetBooksByAuthorId(id);
    for (const auto& book : books) {
        uow->Books().DeleteBookTags(book.GetBookId());
        uow->Books().DeleteBook(book.GetBookId());
    }
    uow->Authors().Delete(id);
    uow->Commit();
}

void UseCasesImpl::EditAuthor(const domain::AuthorId& id, const std::string& new_name) {
    auto uow = unit_factory_.GetUnitOfWork();
    uow->Authors().Edit(id, new_name);
    uow->Commit();
}

domain::Authors UseCasesImpl::GetAllAuthors() {
    auto uow = unit_factory_.GetUnitOfWork();
    return uow->Authors().GetAllAuthors();
}

std::optional<domain::Author> UseCasesImpl::FindAuthorById(const domain::AuthorId& id) {
    auto uow = unit_factory_.GetUnitOfWork();
    return uow->Authors().FindAuthorById(id);
}

std::optional<domain::Author> UseCasesImpl::FindAuthorByName(const std::string& name) {
    auto uow = unit_factory_.GetUnitOfWork();
    return uow->Authors().FindAuthorByName(name);
}

void UseCasesImpl::AddBook(const domain::AuthorId& author_id, const std::string& title, int publication_year,
                           domain::Tags tags, const std::string& author_name) {
    auto uow = unit_factory_.GetUnitOfWork();
    uow->Books().Save({BookId::New(), author_id, title, publication_year, std::move(tags), author_name});
    uow->Commit();
}

void UseCasesImpl::DeleteBook(const domain::BookId& id) {
    auto uow = unit_factory_.GetUnitOfWork();
    uow->Books().DeleteBookTags(id);
    uow->Books().DeleteBook(id);
    uow->Commit();
}

void UseCasesImpl::EditBook(const domain::BookId& id, const std::string& title, int publication_year,
                            const domain::Tags& tags) {
    auto uow = unit_factory_.GetUnitOfWork();
    uow->Books().EditBook(id, title, publication_year, tags);
    uow->Commit();
}

domain::Books UseCasesImpl::GetAllBooks() {
    auto uow = unit_factory_.GetUnitOfWork();
    return uow->Books().GetAllBooks();
}

domain::Books UseCasesImpl::GetBooksByAuthor(const domain::AuthorId& author_id) {
    auto uow = unit_factory_.GetUnitOfWork();
    return uow->Books().GetBooksByAuthorId(author_id);
}

domain::Books UseCasesImpl::GetBooksByTitle(const std::string& title) {
    auto uow = unit_factory_.GetUnitOfWork();
    return uow->Books().GetBooksByTitle(title);
}

}  // namespace app
