# Bookypedia <!-- omit in toc -->

Консольное приложение для управления библиотекой (книги, авторы, теги) на C++20 с хранением в PostgreSQL (libpqxx). Управление через набор терминальных команд; все операции выполняются в отдельных транзакциях.

## Содержание <!-- omit in toc -->
- [Возможности](#возможности)
- [Архитектура](#архитектура)
- [Требования](#требования)
- [Сборка](#сборка)
- [Запуск](#запуск)
- [Поддерживаемые команды](#поддерживаемые-команды)
- [Примеры](#примеры)
- [Лицензия](#лицензия)

## Возможности

* Добавление, редактирование и удаление авторов
* Добавление, редактирование и удаление книг
* Поддержка тегов для каждой книги (ввод списком, нормализация, удаление дублей)
* Просмотр списка авторов и книг; детальная карточка книги
* Автоматическое создание таблиц БД при первом запуске
* Каждая команда выполняется в отдельной транзакции (атомарность, откат при ошибке)

## Архитектура

Приложение построено по принципу разделения слоёв:

* `domain/` — предметная область (классы `Author`, `Book`, типы идентификаторов).
* `postgres/` — доступ к PostgreSQL через библиотеку **libpqxx**.
* `app/` — бизнес-логика и сценарии использования (use cases).
* `menu/` — парсинг и маршрутизация пользовательских команд.
* `ui/` — вывод данных в консоль.
* `util/` — вспомогательные типы и функции (включая UUID-идентификаторы).

<details><summary><strong>Структура проекта</strong></summary>

```bash
├── src
│   ├── app
│   │   ├── unit_of_work.h
│   │   ├── use_cases.h
│   │   ├── use_cases_impl.cpp
│   │   └── use_cases_impl.h
│   ├── domain
│   │   ├── author.cpp
│   │   ├── author_fwd.h
│   │   ├── author.h
│   │   ├── book_fwd.h
│   │   └── book.h
│   ├── menu
│   │   ├── menu.cpp
│   │   └── menu.h
│   ├── postgres
│   │   ├── postgres.cpp
│   │   └── postgres.h
│   ├── ui
│   │   ├── view.cpp
│   │   └── view.h
│   ├── util
│   │   ├── tagged.h
│   │   ├── tagged_uuid.cpp
│   │   └── tagged_uuid.h
│   ├── bookypedia.cpp
│   ├── bookypedia.h
│   └── main.cpp
├── tests
│   ├── tagged_uuid_tests.cpp
│   └── use_case_tests.cpp
├── CMakeLists.txt
├── conanfile.txt
├── LICENSE
└── README.md
```

</details>

## Требования
* C++20
* PostgreSQL 15+
* libpqxx
* Boost 1.78+
* GCC 11+
* CMake 3.11+
* Conan 1.x

## Сборка
```bash
# 1. Создать каталог сборки и перейти в него:
mkdir -p build && cd build
# 2. Установить зависимости:
conan install .. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=Release
# 3. Сгенерировать проект:
cmake .. -DCMAKE_BUILD_TYPE=Release
# 4. Собрать проект:
cmake --build .
```

## Запуск

Перед запуском необходимо задать переменную окружения `BOOKYPEDIA_DB_URL` со строкой подключения к PostgreSQL:

```bash
postgresql://<user>:<password>@<host>:<port>/<dbname>
```
где:
`<user>` — имя пользователя в PostgreSQL,
`<password>` — пароль пользователя,
`<host>` — хост БД (обычно `localhost`),
`<port>` — порт (по умолчанию `5432`),
`<dbname>` — имя базы данных.

**Пример:**
```bash
export BOOKYPEDIA_DB_URL="postgres://postgres:secret@localhost:5432/bookypedia"
```

**Запуск приложения:**

```bash
./bookypedia
```

## Поддерживаемые команды

- [`AddAuthor <name>`](#ex-add-author) — Добавить автора.
- [`AddBook <year> <title>`](#ex-add-book) — Добавить книгу (ввод/выбор автора, теги).
- [`ShowBooks`](#ex-show-books) — Показать книги (title → author → year).
- [`ShowBook [<title>]`](#ex-show-book) — Карточка книги; при дубликатах — выбор.
- [`ShowAuthors`](#ex-show-authors) — Показать авторов (по алфавиту).
- [`ShowAuthorBooks`](#ex-show-author-books) — Книги выбранного автора.
- [`EditBook [<title>]`](#ex-edit-book) — Изменить название/год/теги.
- [`DeleteBook [<title>]`](#ex-delete-book) — Удалить книгу (с выбором).
- [`EditAuthor [<name>]`](#ex-edit-author) — Переименовать автора.
- [`DeleteAuthor [<name>]`](#ex-delete-author) — Удалить автора и его книги/теги.
- `Help` — Справка по командам.

> Пустая строка на шаге выбора — отмена.

## Примеры

<a id="ex-add-author"></a>
<details><summary><strong>AddAuthor</strong></summary>

```

AddAuthor Jack London
AddAuthor Joanne Rowling
ShowAuthors

1. Jack London
2. Joanne Rowling

```
</details>

<a id="ex-add-book"></a>
<details><summary><strong>AddBook</strong></summary>

- Вариант 1: выбор автора из списка + ввод тегов
```

AddBook 1906 White Fang
Enter author name or empty line to select from list:

Select author:
1 Jack London
2 Joanne Rowling
Enter author # or empty line to cancel
1
Enter tags (comma separated):
adventure,  dog,   gold   rush , , dog

```
Нормализация тегов → `adventure, dog, gold rush`.

- Вариант 2: ввод имени автора вручную (автодобавление) + теги
```

AddBook 1998 Harry Potter and the Chamber of Secrets
Enter author name or empty line to select from list:
Joanne Rowling
No author found. Do you want to add Joanne Rowling (y/n)?
y
Enter tags (comma separated):
magic, school, fantasy

```
</details>

<a id="ex-show-books"></a>
<details><summary><strong>ShowBooks</strong></summary>

```

ShowBooks
1 Harry Potter and the Chamber of Secrets by Joanne Rowling, 1998
2 White Fang by Jack London, 1906

```
</details>

<a id="ex-show-book"></a>
<details><summary><strong>ShowBook</strong></summary>

- Карточка книги
```

ShowBook White Fang
Title: White Fang
Author: Jack London
Publication year: 1906
Tags: adventure, dog, gold rush

```

- При нескольких книгах с одним названием — выбор экземпляра
```

ShowBook The Cloud Atlas
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
Enter the book # or empty line to cancel:
2
Title: The Cloud Atlas
Author: Liam Callanan
Publication year: 2004

```
</details>

<a id="ex-show-authors"></a>
<details><summary><strong>ShowAuthors</strong></summary>

```
ShowAuthors

1. Jack London
2. Joanne Rowling
```

> Список отсортирован по алфавиту.
</details>


<a id="ex-show-author-books"></a>
<details><summary><strong>ShowAuthorBooks</strong></summary>

```

ShowAuthorBooks
Select author:
1 Jack London
2 Joanne Rowling
Enter author # or empty line to cancel
1
1 White Fang, 1906

```
</details>

<a id="ex-edit-book"></a>
<details><summary><strong>EditBook</strong></summary>

```

EditBook White Fang
Enter new title or empty line to use the current one (White Fang):

Enter publication year or empty line to use the current one (1906):
1907
Enter tags (current tags: adventure, dog, gold rush):
adventure, wolf, gold rush

```
</details>

<a id="ex-delete-book"></a>
<details><summary><strong>DeleteBook</strong></summary>

```

DeleteBook The Cloud Atlas
1 The Cloud Atlas by David Mitchell, 2004
2 The Cloud Atlas by Liam Callanan, 2004
Enter the book # or empty line to cancel:
1

```
</details>

<a id="ex-edit-author"></a>
<details><summary><strong>EditAuthor</strong></summary>

```

EditAuthor Jack London
Enter new name:
John Griffith Chaney
ShowAuthors
1 John Griffith Chaney
2 Joanne Rowling

```
</details>

<a id="ex-delete-author"></a>
<details><summary><strong>DeleteAuthor</strong></summary>

```

DeleteAuthor Joanne Rowling
ShowAuthors
1 John Griffith Chaney

```
</details>

## Лицензия

MIT — см. файл [LICENSE](LICENSE).
