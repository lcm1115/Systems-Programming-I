        .globl find_course
find_course:
        enter   $0, $0

# Get head of linked list of courses
        movl    courses, %eax

loop:
# If end of list is reached, return NULL
        cmpl    $0, %eax
        je      done

# Compare first character
        movb    8(%ebp), %cl
        movb    10(%eax), %dl
        cmpb    %cl, %dl
        jne     continue

# Compare second character
        movb    12(%ebp), %cl
        movb    11(%eax), %dl
        cmpb    %cl, %dl
        jne     continue

# Compare course ID
        movw    16(%ebp), %cx
        movw    8(%eax), %dx
        cmpw    %cx, %dx
        je      done

# Move to next course and loop
continue:
        movl    (%eax), %eax
        jmp     loop

done:
        leave
        ret
