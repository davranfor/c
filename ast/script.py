# Script example

def foo()
    pow = pow(3, 2);
    print("pow = ", pow, "\n");
end

def bar(var1, var2)
    x = 69;
    print(x, "\n");
return;
    foo();
    print("bar = ", var1, " ", var2, "\n");
end

def square(x)
    return x * x;
end
    
def main()
    print("Starting ...\n");

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

    # Strings promoted to number
    print(pow(trunc("3" + 0.14), 2), "\n");
    print(ceil("4.15"), "\n");

    # Testing print
    print(null, "\t", true, "\t", false, "\n");

    x = 0;
    for (i = 0, i < 10, i += 1)
        x += 1;
    end
    print("x = ", x, "\n");

    name = "Juan"; age = 20;
    print(name, " es ", cond(age > 65, "viejo", cond(age < 15, "joven", "adulto")), "\n");

    print("x = ", x = false, " typeof x = ", typeof(x), "\n");

    x = "prueba2";
    bar("prueba1", x);

    print(square(9), "\n");

    print("Bye\n");
end

