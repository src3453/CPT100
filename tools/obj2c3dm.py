import sys
import struct
import os

def parse_obj(filename):
    vertices = []
    normals = []
    uvs = []
    faces = []

    with open(filename, 'r',encoding='utf-8') as f:
        for line in f:
            if line.startswith('#'): continue
            parts = line.strip().split()
            if not parts: continue

            if parts[0] == 'v':
                vertices.append([float(x) for x in parts[1:4]])
            elif parts[0] == 'vn':
                normals.append([float(x) for x in parts[1:4]])
            elif parts[0] == 'vt':
                uvs.append([float(x) for x in parts[1:3]])
            elif parts[0] == 'f':
                face = []
                for v_str in parts[1:]:
                    v_parts = v_str.split('/')
                    v_idx = int(v_parts[0]) - 1
                    vt_idx = int(v_parts[1]) - 1 if len(v_parts) > 1 and v_parts[1] else -1
                    vn_idx = int(v_parts[2]) - 1 if len(v_parts) > 2 and v_parts[2] else -1
                    face.append((v_idx, vt_idx, vn_idx))
                
                # Triangulate
                for i in range(1, len(face) - 1):
                    faces.append([face[0], face[i], face[i+1]])

    return vertices, normals, uvs, faces

def float_to_q20_12(val):
    return int(val * 4096.0)

def float_to_q1_15(val):
    res = int(val * 32768.0)
    return max(-32768, min(32767, res))

def float_to_q10_6(val):
    res = int(val * 64.0)
    return max(0, min(65535, res))

def write_c3dm(output_filename, vertices, normals, uvs, faces):
    # Flatten vertices for export
    unique_vertices = []
    vertex_map = {}
    indices = []

    for face in faces:
        for v_idx, vt_idx, vn_idx in face:
            key = (v_idx, vt_idx, vn_idx)
            if key not in vertex_map:
                vertex_map[key] = len(unique_vertices)
                
                px, py, pz = vertices[v_idx]
                nx, ny, nz = normals[vn_idx] if vn_idx >= 0 else (0, 0, 0)
                u, v = uvs[vt_idx] if vt_idx >= 0 else (0, 0)
                
                unique_vertices.append({
                    'pos': (px, py, pz),
                    'norm': (nx, ny, nz),
                    'uv': (u, v)
                })
            indices.append(vertex_map[key])

    # Header
    magic = 0x4D443343 # 'C3DM'
    version = 1
    material_count = 0
    mesh_count = 1
    
    # Offsets
    header_size = 36 # 4 + 2 + 2 + 4 + 4 + 4 + 16
    material_table_offset = header_size
    mesh_table_offset = material_table_offset + (material_count * 0) # No materials for now
    
    # Mesh Header
    mesh_header_size = 20 # 4 + 2 + 2 + 4 + 4 + 4 (aligned?) -> struct says:
    # uint32_t magic;           // 'MESH'
    # uint16_t version;
    # uint16_t flags;           // bit0: index32
    # uint32_t vertexCount;
    # uint32_t indexCount;      // 3 の倍数
    # uint32_t materialId;
    # Total: 4+2+2+4+4+4 = 20 bytes
    
    mesh_chunk_header_size = 20
    
    # Calculate total size to determine offsets
    mesh_offset = mesh_table_offset + (mesh_count * 4)
    
    with open(output_filename, 'wb') as f:
        # File Header
        f.write(struct.pack('<IHHIII', magic, version, material_count, mesh_count, material_table_offset, mesh_table_offset))
        f.write(b'\x00' * 16) # reserved
        
        # Mesh Table (offsets)
        f.write(struct.pack('<I', mesh_offset))
        
        # Mesh Chunk
        mesh_magic = 0x4853454D # 'MESH'
        
        use_index32 = len(unique_vertices) > 65535
        flags = 1 if use_index32 else 0
        
        f.write(struct.pack('<IHHIII', mesh_magic, 1, flags, len(unique_vertices), len(indices), 0))
        
        # Vertices
        # struct C3DM_VertexPacked {
        #   int32_t px,py,pz;       // Q20.12
        #   int16_t nx,ny,nz;       // Q1.15
        #   uint16_t u,v;           // Q10.6
        #   uint16_t pad;           // reserved
        # };
        for v in unique_vertices:
            px = float_to_q20_12(v['pos'][0])
            py = float_to_q20_12(v['pos'][1])
            pz = float_to_q20_12(v['pos'][2])
            
            nx = float_to_q1_15(v['norm'][0])
            ny = float_to_q1_15(v['norm'][1])
            nz = float_to_q1_15(v['norm'][2])
            
            u = float_to_q10_6(v['uv'][0])
            v_coord = float_to_q10_6(v['uv'][1])
            
            f.write(struct.pack('<iiihhhHHH', px, py, pz, nx, ny, nz, u, v_coord, 0))
            
        # Indices
        if use_index32:
            for idx in indices:
                f.write(struct.pack('<I', idx))
        else:
            for idx in indices:
                f.write(struct.pack('<H', idx))

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: python obj2c3dm.py input.obj output.c3d")
        sys.exit(1)
    
    input_obj = sys.argv[1]
    output_c3dm = sys.argv[2]
    
    verts, norms, uvs, faces = parse_obj(input_obj)
    write_c3dm(output_c3dm, verts, norms, uvs, faces)
    print(f"Converted {input_obj} to {output_c3dm}")
