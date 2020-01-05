print("=============================");
b = 0;
#b = 0;
a = true;
if (b)
    print(b);
end
if (a == 0)
    print("a == 2");
elif (a >= 0)
    print("a >= 0");
    if (a == 1)
        print("a == 1");
        if (a == 0)
            print("a == 1");
        end
    end
# TODO: Put the complete block here and it fails
else
    print("Other");
end

if (a == 1)
    print("a es = 1");
    if (a == 0)
        print("a == 0");
    end
end

print(a);
print("=============================");
