-- a single line comment

{- a multiline comment
    {- can be nested -}
-}

-- Boolean
true = (True)
false = (False)

-- Int
i1 = (0)
i2 = (42)
i3 = (9000)
i4 = (0xFF)   -- 255 in hexadecimal
i5 = (0x000A) --  10 in hexadecimal

-- Float
f1 = (0)
f2 = (42)
f3 = (3.14)
f4 = (0.1234)
f5 = (6.022e23)   -- == (6.022 * 10^23)
f6 = (6.022e+23)  -- == (6.022 * 10^23)
f7 = (1.602e-19)
--1e3        -- == (1 * 10^3) == 1000

-- Strings
s1 = ('a')
s2 = ("abc")

-- Expressions
calc =
    (test (calc 42 12) (3 4 5) True 3.14)
--True && not (True || False)
--(2 + 4) * (4^2 - 9)
--"abc" ++ "def"

-- Lists
l1 = ([])
l2 = ([1])
l3 = ([1,2,3])

-- Tuples
t1 = (())
t2 = ((1))
t3 = ((1,2,3))

-- Conditionals
cond1 =
    (if powerLevel 9000 then "OVER 9000!!!" else "meh")

cond2 =
    (if eq key 40 then
        add n 1
    else if eq key 38 then
        sub n 1
    else
        n)

-- Case of
case1 n =
    (case n of
        0 -> (1)
        1 -> (test 0)
        2 -> (3)
        _ -> (2))

case2 =
    (case x of
        True -> (1)
        False -> (0))

