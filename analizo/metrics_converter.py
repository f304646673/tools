import pandas as pd
import yaml
import sys

def yaml2md(yaml_file, md_file):
    li = []
    summary = None
    with open(yaml_file, 'r') as f:
        for g in yaml.safe_load_all(f):
            if "_module" in g:
                li.append(g)
            else:
                summary = pd.json_normalize(yaml.safe_load_all(yaml.safe_dump_all([g])))
    df = pd.json_normalize(yaml.safe_load_all(yaml.safe_dump_all(li)))
    df.set_index("_module", inplace=True)
    column_to_move = df.pop("_filename")
    df.insert(len(df.columns), "name", column_to_move)
    with open(md_file, 'w') as f:
        f.write(df.to_markdown())
    
    
if __name__ == "__main__":
    yaml_path = sys.argv[1]
    md_path = sys.argv[2]
    yaml2md(yaml_path, md_path)