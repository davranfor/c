print("=============================");
a = 0;

if (a == 1)
    print("a == 1");
elif (a == 0)
    print("a == 0");
    a = 10;
    if (a == 1)
        print("a == 1");
        a = 10;
    elif (a == 10)
        print("a == 10");
    else
        print(a);
    end
    if (a > 10)
        print("a > 10");
    elif (a > 5)
        print("a > 5");
    end
else
    print(a);
end

print(a);
print("=============================");
