hf_min_version_required 2.15

proc Position {} {
    set x [float X]
    set y [float Y]
    set z [float Z]
    sectionvalue "($x, $y, $z)"
}

proc BoundingBox {} {
    section "Bounding box" {
        section -collapsed  "Center" {
            Position
        }
        section -collapsed "Half extent" {
            Position
        }
    }
}

proc Flags {} {
    set value [uint32]
    section "Flags" {
        if {[expr $value & 0x1] == 0x1} {
            entry "Interleaved" ""
        } elseif {[expr $value & 0x1] == 0x0} {
            entry "Non-interleaved" ""
        }
        if {[expr $value & 0x2] == 0x2} {
            entry "UV" "UNORM_16"
        } elseif {[expr $value & 0x2] == 0x0} {
            entry "UV" "FP_16"
        }
        if {[expr $value & 0x3] == 0x3} {
            entry "Compression" "meshopt"
        } elseif {[expr $value & 0x3] == 0x0} {
            entry "Compression" "None"
        }
    }
}

proc Attribute {name} {
    section $name {
        set position [uint32]
        set offset [uint32]
        if {$position != 0xffffffff} {
            entry "Position" $position
        } else {
            entry "Position" "N/A"
        }
        if {$offset != 0xffffffff} {
            entry "Offset" $offset
        } else {
            entry "Offset" "N/A"
        }
    }
}

proc Part {} {
    uint32 "Index offset"
    uint32 "Index count"
    uint32 "Min index"
    uint32 "Max index"
    uint32 "Material ID"
    BoundingBox
}

proc Material {} {
    set nameLength [uint32]
    cstr "utf8" "Name"
}

little_endian
requires 0 "46 49 4C 41 4D 45 53 48"
bytes 8 "Signature"

uint32 "Version"
set partCount [uint32 "Parts count"]

BoundingBox
Flags
Attribute "Position"
Attribute "Tangent"
Attribute "Color"
Attribute "UV0"
Attribute "UV1"

section "Vertices" {
    uint32 "Count"
    set vertexSize [uint32 "Byte size"]
}

section "Indices" {
    set type [uint32]
    uint32 "Count"
    set indexSize [uint32 "Byte size"]
    if {$type == 0x0} {
        entry "Type" "UINT32"
    } else {
        entry "Type" "UINT16"
    }
}

bytes $vertexSize "Vertex data"
bytes $indexSize "Index data"

section "Parts" {
    for {set i 0} {$i < $partCount} {incr i} {
        section -collapsed $i {
            Part
        }
    }
}

section "Materials" {
    set materialCount [uint32]
    for {set i 0} {$i < $materialCount} {incr i} {
        section -collapsed $i {
            Material
        }
    }
}
