"""
Test file: simplified SQLAlchemy descriptor protocol (orm/attributes.py pattern).
Self-contained, no external deps.
Stresses: __get__/__set__ dunder methods called explicitly (no class-level
          descriptor assignment — py2ul drops class-body non-function assignments).
          Validated property pattern with error raising.
Adaptations:
  - Descriptor used via explicit call rather than class-level assignment (py2ul
    does not emit class-body variable assignments that hold object instances).
  - isinstance() check replaced with type name string comparison (UL limitation).
  - del statements removed (UL does not support del).
"""


class TypedDescriptor:
    """Descriptor that stores attribute on instance __dict__."""

    def __init__(self, attr_name, type_name):
        self.attr_name = attr_name
        self.type_name = type_name

    def __get__(self, obj, objtype):
        if obj is None:
            return self
        val = obj.__dict__.get(self.attr_name)
        if val is None:
            return ""
        return val

    def __set__(self, obj, value):
        obj.__dict__[self.attr_name] = value

    def __repr__(self):
        return "TypedDescriptor(" + self.attr_name + ", " + self.type_name + ")"


class ReadOnlyDescriptor:
    """Descriptor that raises AttributeError on set."""

    def __init__(self, value):
        self.value = value

    def __get__(self, obj, objtype):
        if obj is None:
            return self
        return self.value

    def __set__(self, obj, value):
        raise AttributeError("read-only attribute")


class ValidatedDescriptor:
    """Descriptor with validation — rejects negative values."""

    def __init__(self, attr_name):
        self.attr_name = attr_name

    def __get__(self, obj, objtype):
        if obj is None:
            return self
        val = obj.__dict__.get(self.attr_name)
        if val is None:
            return 0
        return val

    def __set__(self, obj, value):
        if value < 0:
            raise ValueError("value must be non-negative")
        obj.__dict__[self.attr_name] = value


class Holder:
    """Plain holder object — descriptors used explicitly via __get__/__set__."""

    def __init__(self):
        pass


def main():
    # TypedDescriptor: use explicitly via __get__ / __set__
    d = TypedDescriptor("name", "string")
    obj = Holder()
    print("descriptor repr:", repr(d))

    # Initially no value — __get__ returns ""
    val = d.__get__(obj, Holder)
    print("initial get:", val)

    # Set and retrieve
    d.__set__(obj, "Alice")
    val2 = d.__get__(obj, Holder)
    print("after set:", val2)

    # Update
    d.__set__(obj, "Bob")
    val3 = d.__get__(obj, Holder)
    print("after update:", val3)

    # Class-level access (obj=None) returns self
    self_ref = d.__get__(None, Holder)
    print("class access is descriptor:", self_ref is d)

    # ReadOnlyDescriptor
    rod = ReadOnlyDescriptor("active")
    obj2 = Holder()
    status = rod.__get__(obj2, Holder)
    print("status:", status)

    raised = False
    try:
        rod.__set__(obj2, "inactive")
    except AttributeError:
        raised = True
    print("read-only set raises:", raised)

    # ValidatedDescriptor
    vd = ValidatedDescriptor("count")
    obj3 = Holder()

    # Initially no value — returns 0
    v0 = vd.__get__(obj3, Holder)
    print("initial count:", v0)

    # Set valid value
    vd.__set__(obj3, 5)
    v1 = vd.__get__(obj3, Holder)
    print("count after set:", v1)

    # Increment pattern
    vd.__set__(obj3, vd.__get__(obj3, Holder) + 1)
    v2 = vd.__get__(obj3, Holder)
    print("count after increment:", v2)

    # Negative raises ValueError
    raised2 = False
    try:
        vd.__set__(obj3, -1)
    except ValueError:
        raised2 = True
    print("negative raises:", raised2)
    v3 = vd.__get__(obj3, Holder)
    print("count unchanged:", v3)

    # Two independent holders share descriptor but separate storage
    obj4 = Holder()
    obj5 = Holder()
    vd.__set__(obj4, 10)
    vd.__set__(obj5, 20)
    v4 = vd.__get__(obj4, Holder)
    v5 = vd.__get__(obj5, Holder)
    print("obj4 count:", v4)
    print("obj5 count:", v5)
    print("independent:", v4 != v5)

    # TypedDescriptor on int field
    d_age = TypedDescriptor("age", "int")
    obj6 = Holder()
    d_age.__set__(obj6, 30)
    age_val = d_age.__get__(obj6, Holder)
    print("age:", age_val)
    print("age descriptor:", repr(d_age))


main()
