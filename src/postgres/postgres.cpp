#include "postgres.h"

#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator""_zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    work_.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
}

void AuthorRepositoryImpl::Delete(const domain::AuthorId& author_id) {
    work_.exec_params("DELETE FROM authors WHERE id = $1", author_id.ToString());
}

void AuthorRepositoryImpl::Edit(const domain::AuthorId& author_id, const std::string& new_name) {
    work_.exec_params("UPDATE authors SET name = $1 WHERE id = $2", new_name, author_id.ToString());
}

domain::Authors AuthorRepositoryImpl::GetAllAuthors() {
    const pqxx::zview query_text = "SELECT id, name FROM authors ORDER BY name;"_zv;

    domain::Authors authors;
    for (const auto& [id, name] : work_.query<std::string, std::string>(query_text)) {
        authors.emplace_back(domain::AuthorId::FromString(id), name);
    }
    return authors;
}

std::optional<domain::Author> AuthorRepositoryImpl::FindAuthorById(const domain::AuthorId& author_id) {
    const std::string query_text = "SELECT id, name FROM authors WHERE id = " + work_.quote(author_id.ToString()) + ";";

    auto query_result = work_.query01<std::string, std::string>(query_text);

    if (!query_result) {
        return std::nullopt;
    }

    const auto& [id, name] = *query_result;
    return domain::Author{domain::AuthorId::FromString(id), name};
}

std::optional<domain::Author> AuthorRepositoryImpl::FindAuthorByName(const std::string& input_name) {
    const std::string query_text = "SELECT id, name FROM authors WHERE name = " + work_.quote(input_name) + ";";

    auto query_result = work_.query01<std::string, std::string>(query_text);

    if (!query_result) {
        return std::nullopt;
    }

    const auto& [id, name] = *query_result;
    return domain::Author{domain::AuthorId::FromString(id), name};
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    work_.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
)"_zv,
        book.GetBookId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear());

    work_.exec_params("DELETE FROM book_tags WHERE book_id = $1;", book.GetBookId().ToString());
    for (const auto& tag : book.GetTags()) {
        work_.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);", book.GetBookId().ToString(), tag);
    }
}

domain::Books BookRepositoryImpl::GetAllBooks() {
    pqxx::zview query_text = R"(
    SELECT b.id, b.author_id, b.title, b.publication_year, a.name
    FROM books b
    JOIN authors a ON b.author_id = a.id
    ORDER BY b.title;
)"_zv;

    domain::Books books;
    for (const auto& [book_id, author_id, title, publication_year, author_name] :
         work_.query<std::string, std::string, std::string, int, std::string>(query_text)) {
        books.emplace_back(domain::BookId::FromString(book_id), domain::AuthorId::FromString(author_id), title,
                           publication_year, GetBookTags(book_id), author_name);
    }
    return books;
}

domain::Tags BookRepositoryImpl::GetBookTags(const std::string& book_id) const {
    const std::string query_text = "SELECT tag FROM book_tags WHERE book_id = " + work_.quote(book_id) + ";";

    domain::Tags result;
    for (const auto& [tag] : work_.query<std::string>(query_text)) {
        result.push_back(tag);
    }

    return result;
}

domain::Books BookRepositoryImpl::GetBooksByAuthorId(const domain::AuthorId& author_id) {
    const std::string query_text =
        R"(
        SELECT id, author_id, title, publication_year
        FROM books
        WHERE author_id = )" +
        work_.quote(author_id.ToString()) + R"(
        ORDER BY publication_year, title;
    )";

    domain::Books books;
    for (const auto& [book_id, author_id, title, publication_year] :
         work_.query<std::string, std::string, std::string, int>(query_text)) {
        books.emplace_back(domain::BookId::FromString(book_id), domain::AuthorId::FromString(author_id), title,
                           publication_year, GetBookTags(book_id));
    }
    return books;
}

domain::Books BookRepositoryImpl::GetBooksByTitle(const std::string& title) {
    const std::string query_text =
        R"(
        SELECT b.id, b.author_id, b.title, b.publication_year, a.name
        FROM books b
        JOIN authors a ON b.author_id = a.id
        WHERE title = )" +
        work_.quote(title) + R"(
        ORDER BY b.publication_year;
    )";

    domain::Books books;
    for (const auto& [book_id, author_id, title, publication_year, author_name] :
         work_.query<std::string, std::string, std::string, int, std::string>(query_text)) {
        books.emplace_back(domain::BookId::FromString(book_id), domain::AuthorId::FromString(author_id), title,
                           publication_year, GetBookTags(book_id), author_name);
    }
    return books;
}

void BookRepositoryImpl::DeleteBookTags(const domain::BookId& book_id) {
    work_.exec_params("DELETE FROM book_tags WHERE book_id = $1", book_id.ToString());
}

void BookRepositoryImpl::DeleteBook(const domain::BookId& book_id) {
    work_.exec_params("DELETE FROM books WHERE id = $1", book_id.ToString());
}

void BookRepositoryImpl::EditBook(const domain::BookId& book_id, const std::string& title, int publication_year,
                                  const domain::Tags& tags) {
    work_.exec_params("UPDATE books SET title = $2, publication_year = $3 WHERE id = $1;", book_id.ToString(), title,
                      publication_year);
    work_.exec_params("DELETE FROM book_tags WHERE book_id = $1;", book_id.ToString());
    for (const auto& tag : tags) {
        work_.exec_params("INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);", book_id.ToString(), tag);
    }
}

Database::Database(pqxx::connection connection) : connection_{std::move(connection)} {
    pqxx::work work{connection_};

    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);

    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL,
    title varchar(100) NOT NULL,
    publication_year INTEGER NOT NULL
);
)"_zv);

    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id UUID REFERENCES books(id) NOT NULL,
    tag varchar(30) NOT NULL
);
)"_zv);

    work.commit();
}

}  // namespace postgres
