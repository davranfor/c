# Script example

def pepe(a, b)
end

def foo()
    (1) + 1;
    pow = pow(3, 2);
    print("pow = ", pow);
    if (1)
    end
end

def bar(var1, var2)
    x = 69;
    print(x);
    foo();
    print("bar = ", var1, " ", var2);
end

def foobar(a, c)
end

def square(x)
    return x * x;
end

def reverse(x, n)
    if (x == n)
        return;
    end
    reverse(x + 1, n);
    print(x);
end
    
def main()
    print("Starting ...");

    # for 1 to 999 print multiples of 100
    for (i = 1, i < 1e3, i += 1)
        if ((i % 100) == 0)
            print("i = ", i);
        end
    end

    test = 1;
    if (test < 1)
        print("test < 1");
    elif (test > 1)
        print("test > 1");
    else
        print("test = 1");
        # Testing inner statement
        if (1)
            if (0) 0; end
        end
    end

    # Testing parenthesis
    print((3 * 2) + (3 * 2));

    # Strings promoted to number
    print(pow(trunc("3" + 0.14), 2));
    print(ceil("4.15"));

    # Testing print
    print(null, "\t", true, "\t", false);

    x = 0;
    for (i = 0, i < 10, i += 1)
        x += 1;
    end
    print("x = ", x);

    name = "Juan"; age = 20;
    print(name, " es ", cond(age > 65, "viejo", cond(age < 15, "joven", "adulto")));

    print("x = ", x = false, " typeof x = ", typeof(x));

    x = "prueba2";
    bar("prueba1", x);

    print(square(9));

    reverse(0, 10);

    if (1 == 1)
        print("1 == 1");
    end

    if (1 != 1)
        print("1 != 1");
    end

    if (1 == "1")
        print("1 == \"1\"");
    end

    if (1 != "1")
        print("1 != \"1\"");
    end

    print("");

    if (1 === 1)
        print("1 === 1");
    end

    if (1 !== 1)
        print("1 !== 1");
    end

    if (1 === "1")
        print("1 === \"1\"");
    end

    if (1 !== "1")
        print("1 !== \"1\"");
    end

    a = "Hola";
    print(a >= "Hala");

    print("Bye");
end

