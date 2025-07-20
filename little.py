def to_little_endian(number, size):
    """
    Convert a number to little-endian bytes.
    
    Args:
        number (int): The number to convert
        size (int): Number of bytes (1, 2, 4, or 8)
    
    Returns:
        bytes: Little-endian representation
    """
    if not isinstance(number, int) or number < 0:
        raise ValueError("Number must be a positive integer")
    
    if size not in {1, 2, 4, 8}:
        raise ValueError("Size must be 1, 2, 4, or 8 bytes")
    
    max_value = (1 << (size * 8)) - 1
    if number > max_value:
        raise ValueError(f"Number too large for {size}-byte representation")
    
    return number.to_bytes(size, byteorder='little', signed=False)

# Example usage
if __name__ == "__main__":
    test_numbers = [
        (4096, 8),  # 2-byte
    ]
    
    for num, size in test_numbers:
        le_bytes = to_little_endian(num, size)
        print(f"Number: 0x{num:X} ({size}-byte)")
        print(f"Little-endian: {le_bytes}")
        print(f"Hex: {le_bytes.hex()}")
        print()
