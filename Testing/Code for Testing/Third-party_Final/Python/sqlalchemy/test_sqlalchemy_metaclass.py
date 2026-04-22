"""
Test file: simplified SQLAlchemy metaclass-driven class registration
           (orm/decl_api.py DeclarativeMeta pattern).
Self-contained, no external deps.
Stresses: metaclass __new__ / __init__, class registry populated at class
          definition time, column introspection, class hierarchy with metaclass.
Adaptations:
  - metaclass= kwarg is not supported by py2ul; replaced with explicit
    register() calls in a setup function (same result, different mechanism).
  - Column instances stored in class __dict__ replaced by per-instance
    column list built in __init__ (py2ul drops class-body non-function assigns).
  - No actual SQL generation — just column name lists and registry queries.
"""

# Global registry: maps class name -> class info dict
_mapper_registry = {}


class Column:
    """Represents a database column definition."""

    def __init__(self, name, col_type):
        self.name = name
        self.col_type = col_type

    def __repr__(self):
        return "Column(" + self.name + ", " + self.col_type + ")"


class MapperConfig:
    """Holds column/table metadata for a mapped class."""

    def __init__(self, cls_name, tablename, columns):
        self.cls_name = cls_name
        self.tablename = tablename
        self.columns = columns

    def column_names(self):
        names = []
        for col in self.columns:
            names.append(col.name)
        return names

    def column_types(self):
        types = []
        for col in self.columns:
            types.append(col.col_type)
        return types

    def __repr__(self):
        return "MapperConfig(" + self.cls_name + ", table=" + self.tablename + ")"


def register_class(cls_name, tablename, columns):
    """Register a mapped class with its column definitions."""
    config = MapperConfig(cls_name, tablename, columns)
    _mapper_registry[cls_name] = config
    return config


def get_config(cls_name):
    """Retrieve mapper config by class name."""
    if cls_name in _mapper_registry:
        return _mapper_registry[cls_name]
    return None


class UserModel:
    """Simulated SQLAlchemy model for users."""

    def __init__(self, user_id, username, email):
        self.user_id = user_id
        self.username = username
        self.email = email

    def to_dict(self):
        result = {}
        result["user_id"] = str(self.user_id)
        result["username"] = self.username
        result["email"] = self.email
        return result

    def __repr__(self):
        return "UserModel(id=" + str(self.user_id) + ", name=" + self.username + ")"


class ArticleModel:
    """Simulated SQLAlchemy model for articles."""

    def __init__(self, article_id, title, author_id):
        self.article_id = article_id
        self.title = title
        self.author_id = author_id

    def to_dict(self):
        result = {}
        result["article_id"] = str(self.article_id)
        result["title"] = self.title
        result["author_id"] = str(self.author_id)
        return result

    def __repr__(self):
        return "ArticleModel(id=" + str(self.article_id) + ", title=" + self.title + ")"


class TagModel:
    """Simulated SQLAlchemy model for tags."""

    def __init__(self, tag_id, label):
        self.tag_id = tag_id
        self.label = label

    def to_dict(self):
        result = {}
        result["tag_id"] = str(self.tag_id)
        result["label"] = self.label
        return result

    def __repr__(self):
        return "TagModel(id=" + str(self.tag_id) + ", label=" + self.label + ")"


def setup_registry():
    """Simulate what a metaclass __new__ would do at class definition time."""
    register_class("UserModel", "users", [
        Column("user_id", "INTEGER"),
        Column("username", "VARCHAR"),
        Column("email", "VARCHAR"),
    ])
    register_class("ArticleModel", "articles", [
        Column("article_id", "INTEGER"),
        Column("title", "VARCHAR"),
        Column("author_id", "INTEGER"),
    ])
    register_class("TagModel", "tags", [
        Column("tag_id", "INTEGER"),
        Column("label", "VARCHAR"),
    ])


def main():
    setup_registry()

    # Registry populated
    print("UserModel registered:", "UserModel" in _mapper_registry)
    print("ArticleModel registered:", "ArticleModel" in _mapper_registry)
    print("TagModel registered:", "TagModel" in _mapper_registry)
    print("Missing registered:", "Missing" in _mapper_registry)
    print("registry size:", len(_mapper_registry))

    # Config retrieval
    user_config = get_config("UserModel")
    print("user config:", repr(user_config))
    print("user tablename:", user_config.tablename)
    print("user columns:", user_config.column_names())

    article_config = get_config("ArticleModel")
    print("article tablename:", article_config.tablename)
    print("article columns:", article_config.column_names())
    print("article types:", article_config.column_types())

    tag_config = get_config("TagModel")
    print("tag tablename:", tag_config.tablename)
    print("tag column count:", len(tag_config.columns))

    # Column repr
    col = Column("id", "INTEGER")
    print("column repr:", repr(col))

    # Missing config returns None
    missing = get_config("NoSuchModel")
    print("missing config:", missing)

    # Model instances
    u = UserModel(1, "alice", "alice@example.com")
    a = ArticleModel(10, "My Post", 1)
    t = TagModel(100, "python")

    print("user repr:", repr(u))
    print("article repr:", repr(a))
    print("tag repr:", repr(t))

    # to_dict
    ud = u.to_dict()
    print("user dict user_id:", ud["user_id"])
    print("user dict username:", ud["username"])

    ad = a.to_dict()
    print("article dict title:", ad["title"])
    print("article dict author_id:", ad["author_id"])

    td = t.to_dict()
    print("tag dict label:", td["label"])

    # Multiple config queries
    configs = ["UserModel", "ArticleModel", "TagModel"]
    for name in configs:
        cfg = get_config(name)
        print(name + " cols:", len(cfg.columns))


main()
