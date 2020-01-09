print("Starting ...");

for (i = 1, i < 1000, i += 1)
    if ((i % 100) == 0)
        print(i);
    end
end

print("---");

for (i = 1, i < 1000000, i <<= 1)
    print(i);
end

print("Bye");
