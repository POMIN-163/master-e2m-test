import json, re
from jinja2 import Environment, FileSystemLoader
from datetime import datetime

with open("data.json", "r", encoding="utf-8") as f:
    config = json.load(f)

env = Environment(loader=FileSystemLoader("templates"), trim_blocks=True, lstrip_blocks=True)
env.globals['now'] = datetime.now()

# 1. 生成文件
templates = [
    ("ecat_slave.h.j2", "output/ecat_slave_data.h"),
    ("ecat_master.h.j2", "output/ecat_master_data.h"),
]

for tmpl_name, output_path in templates:
    template = env.get_template(tmpl_name)
    output = template.render(od=config["od"])
    with open(output_path, "w", encoding="utf-8", newline='\n') as f:
        f.write(output)
    print(f"Generated: {output_path}")

# 2. 生成并插入 ESI
esi_template = env.get_template("esi.xml.j2")
generated_xml = esi_template.render(od=config["od"])

with open("input.xml", "r", encoding="utf-8") as f:
    content = f.read()

start = "<!-- ==== CUSTOM MODBUS MAPPING OBJECTS START ==== -->"
end   = "<!-- ==== CUSTOM MODBUS MAPPING OBJECTS END ==== -->"
pattern = f"({re.escape(start)}).*?({re.escape(end)})"
replacement = f"{start}\n    {generated_xml.strip()}\n    {end}"

new_content = re.sub(pattern, replacement, content, flags=re.DOTALL)

with open("output/esi.xml", "w", encoding="utf-8", newline='\n') as f:
    f.write(new_content)

print(f"ESI 更新完成！共 {len(config['od'])} 个对象已插入")