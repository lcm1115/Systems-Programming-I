        .globl new_student
new_student:
        enter   $4, $0

# Save ebx
        movl    %ebx, -4(%esp)

# Allocate 20 bytes for student
        pushl   $20
        call    allocate
        popl    %edx

# Check that allocate succeeded
        cmpl    $0, %eax
        je      done

# Save student pointer
        movl    %eax, %ebx

# Calculate length of name
        pushl   12(%ebp)
        call    strlen
        popl    %edx
        addl    $1, %eax

# Allocate memory for string
        pushl   %eax
        call    allocate
        popl    %edx

# Check that allocate succeeded
        cmpl    $0, %eax
        je      allocerr

# Copy given name into student's name
        pushl   12(%ebp)
        pushl   %eax
        call    strcpy
        popl    %edx
        popl    %edx

# Copy sid into student's sid
        movl    %eax, 8(%ebx)
        movl    8(%ebp), %ecx
        movl    %ecx, 12(%ebx)

# Set pointer fields to NULL
        movl    $0, (%ebx)
        movl    $0, 4(%ebx)
        movl    $0, 16(%ebx)

# Move student to return value
        movl    %ebx, %eax

        jmp     done

allocerr:
        pushl   %ebx
        call    unallocate
        popl    %edx
        xor     %eax, %eax

done:
# Restore ebx and return
        movl    -4(%esp), %ebx
        leave
        ret
