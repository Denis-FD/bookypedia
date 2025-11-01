#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"

namespace {

// --- MOCK REPOSITORIES ---
class MockAuthorRepository : public domain::AuthorRepository {
public:
    MockAuthorRepository() = default;

    void Save(const domain::Author& author) override {
        saved_authors_.emplace_back(author);
    }

    domain::Authors GetAllAuthors() override {
        return saved_authors_;
    }

    void Delete(const domain::AuthorId&) override {}
    void Edit(const domain::AuthorId&, const std::string&) override {}

    std::optional<domain::Author> FindAuthorById(const domain::AuthorId& id) override {
        for (const auto& author : saved_authors_)
            if (author.GetId() == id)
                return author;
        return std::nullopt;
    }

    std::optional<domain::Author> FindAuthorByName(const std::string& name) override {
        for (const auto& author : saved_authors_)
            if (author.GetName() == name)
                return author;
        return std::nullopt;
    }

    const domain::Authors& GetSavedAuthors() const noexcept {
        return saved_authors_;
    }

private:
    domain::Authors saved_authors_;
};

class MockBookRepository : public domain::BookRepository {
public:
    MockBookRepository() = default;

    void Save(const domain::Book& book) override {
        saved_books_.emplace_back(book);
    }

    domain::Books GetAllBooks() override {
        return saved_books_;
    }

    domain::Books GetBooksByAuthorId(const domain::AuthorId& id) override {
        domain::Books result;
        for (const auto& book : saved_books_) {
            if (book.GetAuthorId() == id) {
                result.push_back(book);
            }
        }
        return result;
    }

    domain::Books GetBooksByTitle(const std::string&) override {
        return {};
    }
    void DeleteBookTags(const domain::BookId&) override {}
    void DeleteBook(const domain::BookId&) override {}
    void EditBook(const domain::BookId&, const std::string&, int, const domain::Tags&) override {}

    const domain::Books& GetSavedBooks() const noexcept {
        return saved_books_;
    }

private:
    domain::Books saved_books_;
};

// --- MOCK UNIT OF WORK ---
class MockUnitOfWork : public app::UnitOfWork {
public:
    MockUnitOfWork(MockAuthorRepository& authors, MockBookRepository& books) : authors_(authors), books_(books) {}

    domain::AuthorRepository& Authors() override {
        return authors_;
    }
    domain::BookRepository& Books() override {
        return books_;
    }
    void Commit() override {}

private:
    MockAuthorRepository& authors_;
    MockBookRepository& books_;
};

class MockUnitOfWorkFactory : public app::UnitOfWorkFactory {
public:
    MockUnitOfWorkFactory(MockAuthorRepository& authors, MockBookRepository& books)
        : authors_(authors), books_(books) {}

    std::unique_ptr<app::UnitOfWork> GetUnitOfWork() override {
        return std::make_unique<MockUnitOfWork>(authors_, books_);
    }

private:
    MockAuthorRepository& authors_;
    MockBookRepository& books_;
};

// --- FIXTURE ---
struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
};

}  // namespace

// --- TESTS ---
SCENARIO_METHOD(Fixture, "Add Author") {
    GIVEN("UseCasesImpl with mock repositories") {
        MockUnitOfWorkFactory factory{authors, books};
        app::UseCasesImpl use_cases{factory};

        WHEN("Adding a new author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("Author is saved") {
                REQUIRE(authors.GetSavedAuthors().size() == 1);
                CHECK(authors.GetSavedAuthors().at(0).GetName() == author_name);
                CHECK(authors.GetSavedAuthors().at(0).GetId() != domain::AuthorId{});
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Add Book to Author") {
    GIVEN("Existing author in repository") {
        MockUnitOfWorkFactory factory{authors, books};
        app::UseCasesImpl use_cases{factory};

        const auto author_name = "Jack London";
        use_cases.AddAuthor(author_name);
        const auto author_id = authors.GetSavedAuthors().front().GetId();

        WHEN("Adding a book for that author") {
            const auto book_title = "White Fang";
            use_cases.AddBook(author_id, book_title, 1906, {"Adventure"}, author_name);

            THEN("Book appears in repository") {
                REQUIRE(books.GetSavedBooks().size() == 1);
                CHECK(books.GetSavedBooks().at(0).GetTitle() == book_title);
                CHECK(books.GetSavedBooks().at(0).GetAuthorId() == author_id);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "List Authors") {
    GIVEN("Multiple authors added") {
        MockUnitOfWorkFactory factory{authors, books};
        app::UseCasesImpl use_cases{factory};

        use_cases.AddAuthor("Author A");
        use_cases.AddAuthor("Author B");

        WHEN("Retrieving all authors") {
            auto all_authors = authors.GetAllAuthors();

            THEN("All authors are returned") {
                REQUIRE(all_authors.size() == 2);
                CHECK(all_authors.at(0).GetName() == "Author A");
                CHECK(all_authors.at(1).GetName() == "Author B");
            }
        }
    }
}
