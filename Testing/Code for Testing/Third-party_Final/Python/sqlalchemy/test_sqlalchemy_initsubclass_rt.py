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

class Base:
    def __init__(self):
        self.tablename = ''
        self.primary_key = ''
    def get_tablename(self):
        return self.tablename
    def get_primary_key(self):
        return self.primary_key
    def __repr__(self):
        return (((self.__class__.__name__ + '(tablename=') + self.tablename) + ')')

class User(Base):
    def __init__(self, user_id, username):
        self.tablename = 'users'
        self.primary_key = 'user_id'
        self.user_id = user_id
        self.username = username
    def __repr__(self):
        return (((('User(id=' + str(self.user_id)) + ', name=') + self.username) + ')')

class Article(Base):
    def __init__(self, article_id, title):
        self.tablename = 'articles'
        self.primary_key = 'article_id'
        self.article_id = article_id
        self.title = title
    def __repr__(self):
        return (((('Article(id=' + str(self.article_id)) + ', title=') + self.title) + ')')

class Comment(Base):
    def __init__(self, comment_id, body):
        self.tablename = 'comments'
        self.primary_key = 'comment_id'
        self.comment_id = comment_id
        self.body = body
    def __repr__(self):
        return (('Comment(id=' + str(self.comment_id)) + ')')

class AdminUser(User):
    def __init__(self, admin_id, username, level):
        self.tablename = 'admin_users'
        self.primary_key = 'admin_id'
        self.admin_id = admin_id
        self.username = username
        self.user_id = admin_id
        self.level = level
    def __repr__(self):
        return (((('AdminUser(id=' + str(self.admin_id)) + ', level=') + str(self.level)) + ')')

_registry = {}

def register_all():
    global _registry
    _registry['User'] = User
    _registry['Article'] = Article
    _registry['Comment'] = Comment
    _registry['AdminUser'] = AdminUser

def lookup(name):
    global _registry
    if (name in _registry):
        return _registry[name]
    return None

def main():
    register_all()
    print(((str('User in registry:') + ' ') + str(('User' in _registry))))
    print(((str('Article in registry:') + ' ') + str(('Article' in _registry))))
    print(((str('Comment in registry:') + ' ') + str(('Comment' in _registry))))
    print(((str('AdminUser in registry:') + ' ') + str(('AdminUser' in _registry))))
    print(((str('Base in registry:') + ' ') + str(('Base' in _registry))))
    print(((str('registry size:') + ' ') + str(len(_registry))))
    u = User(1, 'alice')
    a = Article(10, 'Hello World')
    c = Comment(100, 'Great post')
    admin = AdminUser(99, 'root', 5)
    print(((str('User.tablename:') + ' ') + str(u.tablename)))
    print(((str('Article.tablename:') + ' ') + str(a.tablename)))
    print(((str('Comment.tablename:') + ' ') + str(c.tablename)))
    print(((str('AdminUser.tablename:') + ' ') + str(admin.tablename)))
    print(((str('User.primary_key:') + ' ') + str(u.primary_key)))
    print(((str('AdminUser.primary_key:') + ' ') + str(admin.primary_key)))
    print(((str('user repr:') + ' ') + str(repr(u))))
    print(((str('article repr:') + ' ') + str(repr(a))))
    print(((str('comment repr:') + ' ') + str(repr(c))))
    print(((str('admin repr:') + ' ') + str(repr(admin))))
    print(((str('AdminUser inherits User:') + ' ') + str(issubclass(AdminUser, User))))
    print(((str('User inherits Base:') + ' ') + str(issubclass(User, Base))))
    print(((str('AdminUser inherits Base:') + ' ') + str(issubclass(AdminUser, Base))))
    print(((str('u is User:') + ' ') + str(isinstance(u, User))))
    print(((str('admin is User:') + ' ') + str(isinstance(admin, User))))
    print(((str('admin is AdminUser:') + ' ') + str(isinstance(admin, AdminUser))))
    print(((str('u is AdminUser:') + ' ') + str(isinstance(u, AdminUser))))
    UserClass = lookup('User')
    print(((str('lookup User found:') + ' ') + str((UserClass != None))))
    ArticleClass = lookup('Article')
    print(((str('lookup Article found:') + ' ') + str((ArticleClass != None))))
    missing = lookup('Missing')
    print(((str('lookup Missing:') + ' ') + str(missing)))
    FoundUser = lookup('User')
    u2 = FoundUser(2, 'bob')
    print(((str('from registry:') + ' ') + str(repr(u2))))
    print(((str('admin.username:') + ' ') + str(admin.username)))
    print(((str('admin.level:') + ' ') + str(admin.level)))
    print(((str('get_tablename:') + ' ') + str(u.get_tablename())))
    print(((str('get_primary_key:') + ' ') + str(u.get_primary_key())))
    return 0


if __name__ == "__main__":
    main()
