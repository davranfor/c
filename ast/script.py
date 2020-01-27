print("Starting ...\n");

def foo()
    print("foo\n");
end

def bar()
    print("bar\n");
end

# for 1 to 999 print multiples of 100
for (i = 1, i < 1e3, i += 1)
    if ((i % 100) == 0)
        print("i = ", i, "\n");
    end
end

test = 1;
if (test < 1)
    print("test < 1\n");
elif (test > 1)
    print("test > 1\n");
else
    print("test = 1\n");
    # Testing inner statement
    if (1)
        if (0) 0; end
    end
end

# Testing parenthesis
print((3 * 2) + (3 * 2) , "\n");

# Strings are promoted to number
print(pow(trunc(("3" + 0.14)), 2), "\n");
print(ceil("4.15"), "\n");

# Testing print
print(null, " ", true, " ", false, "\n");

x = 0;
for (i = 0, i < 10, i += 1)
    x += 1;
end
print("x = ", x, "\n");

name = "Juan";
age = 20;
print(name, " es ", cond(age > 65, "viejo", cond(age < 15, "joven", "adulto")), "\n");

print(foo() + 1, "\n");
bar();

print("Bye\n");

