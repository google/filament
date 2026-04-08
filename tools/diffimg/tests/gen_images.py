import math

def write_ppm(filename, width, height, pixels):
    with open(filename, 'w') as f:
        f.write(f"P3\n{width} {height}\n255\n")
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[y][x]
                f.write(f"{r} {g} {b} ")
            f.write("\n")

def create_ref():
    pixels = [[(0,0,0) for _ in range(64)] for _ in range(64)]
    for y in range(64):
        for x in range(64):
            dx = x - 31.5
            dy = y - 31.5
            if math.sqrt(dx*dx + dy*dy) < 15:
                pixels[y][x] = (255, 255, 255)
    return pixels

def create_shifted():
    pixels = [[(0,0,0) for _ in range(64)] for _ in range(64)]
    for y in range(64):
        for x in range(64):
            dx = (x - 1) - 31.5
            dy = y - 31.5
            if math.sqrt(dx*dx + dy*dy) < 15:
                pixels[y][x] = (255, 255, 255)
    return pixels

def create_dithered():
    pixels = create_ref()
    for y in range(64):
        for x in range(64):
            r, g, b = pixels[y][x]
            if r == 255:
                if (x + y) % 2 == 0:
                    pixels[y][x] = (200, 200, 200)
                else:
                    pixels[y][x] = (255, 255, 255)
    return pixels

if __name__ == '__main__':
    write_ppm('ref.ppm', 64, 64, create_ref())
    write_ppm('cand_shift.ppm', 64, 64, create_shifted())
    write_ppm('cand_blur.ppm', 64, 64, create_dithered())
    print("PPM Images generated successfully.")
