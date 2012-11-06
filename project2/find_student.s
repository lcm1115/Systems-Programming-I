        .globl find_student
find_student:
        enter   $0, $0

# Get front of linked list of students
        movl    students, %eax
        movl    8(%ebp), %ecx

loop:
# If end of list is reached, return NULL
        cmpl    $0, %eax
        je      done

# Get sid of current node, if equal return that node
        movl    12(%eax), %edx
        cmpl    %edx, %ecx
        je      done

# Move to next node and iterate again
        movl    (%eax), %eax
        jmp     loop

done:
        leave
        ret
