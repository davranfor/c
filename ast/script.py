print("Starting ...\n");

# for 1 to 999 print multiples of 100
for (i = 1, i < 1e3, i += 1)
    if ((i % 100) == 0)
        print("i = ", i, "\n");
    end
end

test = 1;
if (test == 0)
    print("test == 0\n");
elif (test > 1)
    print("test > 1\n");
else
    print("test == 1\n");
    if (0)
        0;
    end
end

print((3 * 2) + (3 * 2) , "\n");
print(pow(trunc(("3" + 0.14)), 2), "\n");
print(ceil("4.15"), "\n");
print(null, "\n");

x = 0;
for (i = 0, i < 10, i += 1)
    x += 1;
end
print("x = ", x, "\n");

print("Bye\n");

