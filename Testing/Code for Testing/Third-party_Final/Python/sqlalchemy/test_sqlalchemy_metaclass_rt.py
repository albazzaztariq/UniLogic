# @dr python_compat = true


def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)

# python_compat type aliases (UL keywords remapped by py2ul)
_py_list = list
_py_map = dict
_py_type = type
_py_int = int
_py_float = float
_py_string = str
_py_bool = bool
_py_object = object

class Column:
    def __init__(self, name, col_type):
        self.name = name
        self.col_type = col_type
    def __repr__(self):
        return (((('Column(' + self.name) + ', ') + self.col_type) + ')')

class MapperConfig:
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
        return (((('MapperConfig(' + self.cls_name) + ', table=') + self.tablename) + ')')

class UserModel:
    def __init__(self, user_id, username, email):
        self.user_id = user_id
        self.username = username
        self.email = email
    def to_dict(self):
        result = {}
        result['user_id'] = str(self.user_id)
        result['username'] = self.username
        result['email'] = self.email
        return result
    def __repr__(self):
        return (((('UserModel(id=' + str(self.user_id)) + ', name=') + self.username) + ')')

class ArticleModel:
    def __init__(self, article_id, title, author_id):
        self.article_id = article_id
        self.title = title
        self.author_id = author_id
    def to_dict(self):
        result = {}
        result['article_id'] = str(self.article_id)
        result['title'] = self.title
        result['author_id'] = str(self.author_id)
        return result
    def __repr__(self):
        return (((('ArticleModel(id=' + str(self.article_id)) + ', title=') + self.title) + ')')

class TagModel:
    def __init__(self, tag_id, label):
        self.tag_id = tag_id
        self.label = label
    def to_dict(self):
        result = {}
        result['tag_id'] = str(self.tag_id)
        result['label'] = self.label
        return result
    def __repr__(self):
        return (((('TagModel(id=' + str(self.tag_id)) + ', label=') + self.label) + ')')

_mapper_registry = {}

def register_class(cls_name, tablename, columns):
    global _mapper_registry
    config = MapperConfig(cls_name, tablename, columns)
    _mapper_registry[cls_name] = config
    return config

def get_config(cls_name):
    global _mapper_registry
    if (cls_name in _mapper_registry):
        return _mapper_registry[cls_name]
    return None

def setup_registry():
    register_class('UserModel', 'users', [Column('user_id', 'INTEGER'), Column('username', 'VARCHAR'), Column('email', 'VARCHAR')])
    register_class('ArticleModel', 'articles', [Column('article_id', 'INTEGER'), Column('title', 'VARCHAR'), Column('author_id', 'INTEGER')])
    register_class('TagModel', 'tags', [Column('tag_id', 'INTEGER'), Column('label', 'VARCHAR')])

def main():
    setup_registry()
    print(((str('UserModel registered:') + ' ') + str(('UserModel' in _mapper_registry))))
    print(((str('ArticleModel registered:') + ' ') + str(('ArticleModel' in _mapper_registry))))
    print(((str('TagModel registered:') + ' ') + str(('TagModel' in _mapper_registry))))
    print(((str('Missing registered:') + ' ') + str(('Missing' in _mapper_registry))))
    print(((str('registry size:') + ' ') + str(len(_mapper_registry))))
    user_config = get_config('UserModel')
    print(((str('user config:') + ' ') + str(repr(user_config))))
    print(((str('user tablename:') + ' ') + str(user_config.tablename)))
    print(((str('user columns:') + ' ') + str(user_config.column_names())))
    article_config = get_config('ArticleModel')
    print(((str('article tablename:') + ' ') + str(article_config.tablename)))
    print(((str('article columns:') + ' ') + str(article_config.column_names())))
    print(((str('article types:') + ' ') + str(article_config.column_types())))
    tag_config = get_config('TagModel')
    print(((str('tag tablename:') + ' ') + str(tag_config.tablename)))
    print(((str('tag column count:') + ' ') + str(len(tag_config.columns))))
    col = Column('id', 'INTEGER')
    print(((str('column repr:') + ' ') + str(repr(col))))
    missing = get_config('NoSuchModel')
    print(((str('missing config:') + ' ') + str(missing)))
    u = UserModel(1, 'alice', 'alice@example.com')
    a = ArticleModel(10, 'My Post', 1)
    t = TagModel(100, 'python')
    print(((str('user repr:') + ' ') + str(repr(u))))
    print(((str('article repr:') + ' ') + str(repr(a))))
    print(((str('tag repr:') + ' ') + str(repr(t))))
    ud = u.to_dict()
    print(((str('user dict user_id:') + ' ') + str(ud['user_id'])))
    print(((str('user dict username:') + ' ') + str(ud['username'])))
    ad = a.to_dict()
    print(((str('article dict title:') + ' ') + str(ad['title'])))
    print(((str('article dict author_id:') + ' ') + str(ad['author_id'])))
    td = t.to_dict()
    print(((str('tag dict label:') + ' ') + str(td['label'])))
    configs = ['UserModel', 'ArticleModel', 'TagModel']
    for name in configs:
        cfg = get_config(name)
        print(((str((name + ' cols:')) + ' ') + str(len(cfg.columns))))
    return 0


if __name__ == "__main__":
    main()
