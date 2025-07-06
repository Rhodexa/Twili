import bpy

# Select your LUT mesh object
obj = bpy.data.objects['YourLUTMesh']

# Grab vertices in order (assuming ordered along X)
verts = [v.co for v in obj.data.vertices]
verts.sort(key=lambda v: v.x)

# Output LUT
max_val = 0x1FFF
lut = []

for v in verts:
    y = max(0.0, min(1.0, v.y))  # clamp to 0-1 range
    val = round(y * max_val)
    lut.append(val)

# Format as C array
print("unsigned int gamma_lut[] = {\n  ", end="")
for i, val in enumerate(lut):
    print(f"0x{val:04X}", end=", " if i < len(lut) - 1 else "")
    if (i + 1) % 8 == 0:
        print("\n  ", end="")
print("\n};")
