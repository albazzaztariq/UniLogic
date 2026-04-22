"""
Test file: simplified SQLAlchemy class registry / hierarchy pattern
           (orm/decl_api.py and orm/registry.py pattern).
Self-contained, no external deps.
Stresses: class hierarchy, multi-level subclassing, isinstance/issubclass,
          registry dict, class instantiation from registry lookup.
Adaptations:
  - __init_subclass__ removed (py2ul emits as regular method; classmethod
    protocol broken in codegen).
  - Class-level str/int field assignments dropped by py2ul (known limitation).
    Moved tablename/primary_key into each class __init__ to survive round-trip.
  - Multiple inheritance removed (UL supports single inheritance only).
"""

# Module-level registry: maps class name -> class
_registry = {}


class Base:
    """Base class for all mapped entities."""

    def __init__(self):
        self.tablename = ""
        self.primary_key = ""

    def get_tablename(self):
        return self.tablename

    def get_primary_key(self):
        return self.primary_key

    def __repr__(self):
        return self.__class__.__name__ + "(tablename=" + self.tablename + ")"


class User(Base):
    def __init__(self, user_id, username):
        self.tablename = "users"
        self.primary_key = "user_id"
        self.user_id = user_id
        self.username = username

    def __repr__(self):
        return "User(id=" + str(self.user_id) + ", name=" + self.username + ")"


class Article(Base):
    def __init__(self, article_id, title):
        self.tablename = "articles"
        self.primary_key = "article_id"
        self.article_id = article_id
        self.title = title

    def __repr__(self):
        return "Article(id=" + str(self.article_id) + ", title=" + self.title + ")"


class Comment(Base):
    def __init__(self, comment_id, body):
        self.tablename = "comments"
        self.primary_key = "comment_id"
        self.comment_id = comment_id
        self.body = body

    def __repr__(self):
        return "Comment(id=" + str(self.comment_id) + ")"


class AdminUser(User):
    """Subclass of User."""

    def __init__(self, admin_id, username, level):
        self.tablename = "admin_users"
        self.primary_key = "admin_id"
        self.admin_id = admin_id
        self.username = username
        self.user_id = admin_id
        self.level = level

    def __repr__(self):
        return "AdminUser(id=" + str(self.admin_id) + ", level=" + str(self.level) + ")"


def register_all():
    """Populate registry with all known classes."""
    _registry["User"] = User
    _registry["Article"] = Article
    _registry["Comment"] = Comment
    _registry["AdminUser"] = AdminUser


def lookup(name):
    """Look up a class by name from the registry."""
    if name in _registry:
        return _registry[name]
    return None


def main():
    # Populate registry
    register_all()

    # Registry checks
    print("User in registry:", "User" in _registry)
    print("Article in registry:", "Article" in _registry)
    print("Comment in registry:", "Comment" in _registry)
    print("AdminUser in registry:", "AdminUser" in _registry)
    print("Base in registry:", "Base" in _registry)
    print("registry size:", len(_registry))

    # Instance creation
    u = User(1, "alice")
    a = Article(10, "Hello World")
    c = Comment(100, "Great post")
    admin = AdminUser(99, "root", 5)

    # tablename via instance
    print("User.tablename:", u.tablename)
    print("Article.tablename:", a.tablename)
    print("Comment.tablename:", c.tablename)
    print("AdminUser.tablename:", admin.tablename)

    # primary_key via instance
    print("User.primary_key:", u.primary_key)
    print("AdminUser.primary_key:", admin.primary_key)

    print("user repr:", repr(u))
    print("article repr:", repr(a))
    print("comment repr:", repr(c))
    print("admin repr:", repr(admin))

    # Inheritance: AdminUser inherits from User
    print("AdminUser inherits User:", issubclass(AdminUser, User))
    print("User inherits Base:", issubclass(User, Base))
    print("AdminUser inherits Base:", issubclass(AdminUser, Base))

    # Instance type
    print("u is User:", isinstance(u, User))
    print("admin is User:", isinstance(admin, User))
    print("admin is AdminUser:", isinstance(admin, AdminUser))
    print("u is AdminUser:", isinstance(u, AdminUser))

    # Lookup by name
    UserClass = lookup("User")
    print("lookup User found:", UserClass is not None)
    ArticleClass = lookup("Article")
    print("lookup Article found:", ArticleClass is not None)
    missing = lookup("Missing")
    print("lookup Missing:", missing)

    # Instantiate from registry
    FoundUser = lookup("User")
    u2 = FoundUser(2, "bob")
    print("from registry:", repr(u2))

    # AdminUser instance fields
    print("admin.username:", admin.username)
    print("admin.level:", admin.level)

    # get_tablename via inherited method
    print("get_tablename:", u.get_tablename())
    print("get_primary_key:", u.get_primary_key())


main()
