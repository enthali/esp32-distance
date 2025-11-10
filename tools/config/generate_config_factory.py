#!/usr/bin/env python3
"""
Generate C factory defaults code from config_schema.json

This script parses the JSON configuration schema and generates C code
that initializes all configuration parameters with their default values.

Usage: generate_config_factory.py <schema_file> <output_file>
"""

import json
import sys
import os
from pathlib import Path


def generate_factory_defaults(schema_file: str, output_file: str) -> None:
    """Generate config_factory_generated.c from JSON schema"""
    
    # Parse JSON schema
    try:
        with open(schema_file, 'r', encoding='utf-8') as f:
            schema = json.load(f)
    except FileNotFoundError:
        print(f"ERROR: Schema file not found: {schema_file}", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"ERROR: Invalid JSON in {schema_file}: {e}", file=sys.stderr)
        sys.exit(1)
    
    # Validate required fields
    if 'parameters' not in schema:
        print("ERROR: Schema missing 'parameters' array", file=sys.stderr)
        sys.exit(1)
    
    # Build structured JSON array: [{key, type, value}, ...]
    config_entries = []
    for field in schema['parameters']:
        key = field['key']
        ftype = field['type']
        default = field['default']
        
        # Build JSON object for this parameter
        entry = {
            'key': key,
            'type': ftype,
            'value': default
        }
        config_entries.append(entry)
    
    # Convert to compact JSON string
    json_defaults = json.dumps(config_entries, separators=(',', ':'), ensure_ascii=False)
    
    # Generate C code with embedded JSON
    lines = []
    lines.append("// Auto-generated from config_schema.json - DO NOT EDIT MANUALLY")
    lines.append("// Generator: tools/generate_config_factory.py")
    lines.append("")
    lines.append("#include \"config_manager.h\"")
    lines.append("#include \"esp_log.h\"")
    lines.append("")
    lines.append("static const char *TAG = \"config_factory\";")
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Write factory default values to NVS")
    lines.append(" * ")
    lines.append(" * This function is auto-generated from config_schema.json.")
    lines.append(" * It writes all default configuration values to NVS storage")
    lines.append(" * using the bulk JSON API with structured format: [{key, type, value}, ...]")
    lines.append(" */")
    lines.append("void config_write_factory_defaults(void)")
    lines.append("{")
    lines.append("    ESP_LOGI(TAG, \"Writing factory defaults to NVS...\");")
    lines.append("")
    lines.append("    // Structured JSON array with factory defaults")
    lines.append(f"    const char *factory_json = \"{json_defaults.replace(chr(34), chr(92) + chr(34))}\";")
    lines.append("")
    lines.append("    esp_err_t ret = config_set_all_from_json(factory_json);")
    lines.append("    if (ret == ESP_OK) {")
    lines.append("        ESP_LOGI(TAG, \"Factory defaults written successfully\");")
    lines.append("    } else {")
    lines.append("        ESP_LOGE(TAG, \"Failed to write factory defaults: %s\", esp_err_to_name(ret));")
    lines.append("    }")
    lines.append("}")
    lines.append("")
    
    # Write output file
    try:
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        print(f"Generated: {output_file}")
    except IOError as e:
        print(f"ERROR: Failed to write output file: {e}", file=sys.stderr)
        sys.exit(1)


def main():
    """Main entry point"""
    if len(sys.argv) != 3:
        print("Usage: generate_config_factory.py <schema_file> <output_file>", file=sys.stderr)
        sys.exit(1)
    
    schema_file = sys.argv[1]
    output_file = sys.argv[2]
    
    generate_factory_defaults(schema_file, output_file)


if __name__ == '__main__':
    main()
