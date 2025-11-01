#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>

#include "../app/unit_of_work.h"
#include "../domain/author.h"
#include "../domain/book.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::work& work) : work_{work} {}

    void Save(const domain::Author& author) override;
    void Delete(const domain::AuthorId& author_id) override;
    void Edit(const domain::AuthorId& author_id, const std::string& new_name) override;

    domain::Authors GetAllAuthors() override;
    std::optional<domain::Author> FindAuthorById(const domain::AuthorId& author_id) override;
    std::optional<domain::Author> FindAuthorByName(const std::string&) override;

private:
    pqxx::work& work_;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(pqxx::work& work) : work_{work} {}

    void Save(const domain::Book& book) override;
    domain::Books GetAllBooks() override;
    domain::Books GetBooksByAuthorId(const domain::AuthorId& author_id) override;
    domain::Books GetBooksByTitle(const std::string& title) override;
    void DeleteBookTags(const domain::BookId& book_id) override;
    void DeleteBook(const domain::BookId& book_id) override;
    void EditBook(const domain::BookId& book_id, const std::string& title, int publication_year,
                  const domain::Tags& tags) override;

private:
    pqxx::work& work_;

    domain::Tags GetBookTags(const std::string& book_id) const;
};

class UnitOfWorkImpl : public app::UnitOfWork {
public:
    explicit UnitOfWorkImpl(pqxx::connection& connection) : work_{connection}, authors_{work_}, books_{work_} {}

    domain::AuthorRepository& Authors() override {
        return authors_;
    }

    domain::BookRepository& Books() override {
        return books_;
    }

    void Commit() override {
        work_.commit();
    }

private:
    pqxx::work work_;
    AuthorRepositoryImpl authors_;
    BookRepositoryImpl books_;
};

class Database : public app::UnitOfWorkFactory {
public:
    explicit Database(pqxx::connection connection);

    app::UnitOfWorkPtr GetUnitOfWork() override {
        return std::make_unique<UnitOfWorkImpl>(connection_);
    }

private:
    pqxx::connection connection_;
};

}  // namespace postgres
