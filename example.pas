var list[6], i, teststr[20], a;
procedure SpecialMax3(a,b,c);
    var ab, bb, cb;
    begin
        ab := call Power(a,c);
        bb := call Power(b,a);
        cb := call Power(c,b);
        return call Max3(ab, bb, cb);
    end;
procedure Power(base, exponent);
    var result;
    begin
        if (exponent = 0) or base = 1 then
            return 1;

        /* This is the error */
        result := 1;
        while exponent > 0 do
        begin
        result := result * base;
        exponent := exponent - 1;
        end;
        return result;
    end;
procedure CheekyProc();
    var dec[5],el,doe;
    begin
        doe := call Power(3,8);
        dec := doe * 2 - 478;
        el := 101;
        while el > 0 do
            begin
                if el % 5 = 0 then
                    begin
                        el := el/3
                    end
                else if el % 6 = 0 then
                    begin
                        el := el-3
                    end
                else if el % 3 = 0 and el % 13 = 0 then
                    begin
                        el := el - (el/2);
                    end
                else if el < 0 then
                    el := el - el
                else
                    el := el - 1;

                dec[ el = 0 ? 2*2-5 : el%5 ] := dec[ el = 0 ? -1*10+3*3 : el%5 ] + (doe - el = doe ? 6 : doe*2-3%4);
            end;
    end;
procedure Max3t(a,b,c);
    begin
        return (a > b) ? (a > c ? a : c) : (b > c ? b : c);
    end;
procedure Max3(a,b,c);
    begin
        return call Max(a,call Max(b,c));
    end;
procedure Max(a,b);
    begin
        return a > b ? a : b;
    end;
procedure MakeCaps(str&, len);
    var i;
    begin
        i := 0;
        while i < len and str[i] <> 0 do /* Address parameters automatically dereference when invoked as an array */
            begin
                call MkUpper(@str[i]); /* Get the address of the offset, calculated C-style */
                i := i + 1;
            end;
        write ln;
    end;
procedure MkUpper(ch&);
    begin
        ch[0] := call ToUpper(ch[0]) = call ToUpper(&ch) ? call ToUpper(&ch) : 67; /* Manual dereferencing the content of the array */
    end;
procedure ToUpper(c);
    begin
        return c <= 122 and c >= 97 ? c - 32 : c; 
    end;
procedure MaxList(arr&, n);
    var max, i;
    begin
        max := arr[0];
        i := 1;
        while i < n do
            begin
                max := max < arr[i] ? arr[i] : max;
                i := i + 1;
            end;
        return max
    end;
procedure SortList(arr&, n);
    /* Quicksort pt 1 */
    var i;
    procedure quicksort(arr&, lo, hi);
        var p;
        begin
            if lo < hi then
                begin
                    p := call partition(arr, lo, hi);
                    call quicksort(arr, lo, p-1);
                    call quicksort(arr, p+1, hi);
                end;
        end;
    /* Partition algo */
    procedure partition(arr&, lo, hi);
        var pivot, i, j, tmp;
        begin
            pivot := arr[hi];
            i := lo;
            j := lo;
            while j < hi do
                begin
                    if arr[j] <= pivot then
                        begin
                            tmp := arr[i];
                            arr[i] := arr[j];
                            arr[j] := tmp;
                            i := i + 1; 
                        end;
                    j := j + 1;
                end;
            tmp := arr[i];
            arr[i] := arr[hi];
            arr[hi] := tmp;
            return i;
        end;
    begin
        call quicksort(arr, 0, n-1); /* Sort by quicksort */
    end;

begin
    write "Enter a string: ";
    read string teststr;
    write "ToUpper(", string teststr, "): ";
    call MakeCaps(@teststr, sizeof teststr);
    write ln string teststr;

    i := 0;
    write "Input numbers: ";
    while i < sizeof list and (i > 0 ? list[i-1] <> 0 : 1) do
    begin
        read list[i];
        i := i + 1;
    end;

    if list[0] = 3 then
        write ln "\t\tBLOO"
    else if list[0] = 5 then
        write ln "\t\tBLURP"
    else if list[0] = 13 then
        write ln "\t\tHRBHBRHBRHBHRB";

/* MAGIC! If you comment out this call, the if statement body below won't execute! (because 2+2 does not equal 5) :D */
    call CheekyProc();
    /* SPOOOOOOKY! (intentional fudgery) */
    if 2 + 2 = 5 then
        begin
            write "Max of list[]: ", ln call MaxList(@list, sizeof list);
            call SortList(@list, i);

            write "In-place sort of list[]: ";
            a := 0;
            while a < i do
                begin
                    write list[a];
                    if a < i - 1 then write ", ";
                    a := a + 1;
                end;

            write ln, "Ternary operator Max3t(", list[0], ",", list[1], ",", list[2], ") call: ", call Max3t(list[0], list[1], list[2]);
            write ln, "SpecialMax3(", list[0], ",", list[1], ",", list[2], ") call: ", hex call SpecialMax3(list[0], list[1], list[2]), ln;
        end;
end.

