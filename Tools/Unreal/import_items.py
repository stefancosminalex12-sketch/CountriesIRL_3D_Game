"""
Import the item tables (Data/Items/*.csv) as Data Tables of FCIRLItemRow, one per file:
Data/Items/Items_England1455.csv -> /Game/CrownsAndCommoners/Items/DT_Items_England1455. Safe to run again after
editing a CSV (rows are replaced).
    in the editor:  py "C:/Dev/CrownsAndCommoners/Tools/Unreal/import_items.py"
    headless:       UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<this file>"
New tables also need a line in Config/DefaultGame.ini ([/Script/CrownsAndCommoners.CIRLItemSettings] +ItemTables=...).
"""
import glob
import os
import unreal

PROJECT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
FOLDER = "/Game/CrownsAndCommoners/Items"
row_struct = unreal.load_object(None, "/Script/CrownsAndCommoners.CIRLItemRow")

for csv in sorted(glob.glob(PROJECT + "Data/Items/*.csv")):
    name = "DT_" + os.path.splitext(os.path.basename(csv))[0]
    path = "%s/%s" % (FOLDER, name)
    table = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if not table:
        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", row_struct)
        table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, FOLDER, unreal.DataTable, factory)
    ok = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(table, csv.replace("\\", "/"))
    rows = unreal.DataTableFunctionLibrary.get_data_table_row_names(table)
    unreal.EditorAssetLibrary.save_loaded_asset(table, False)
    print("ITEMS_IMPORT", name, "ok" if ok else "FAILED", len(rows), "rows")
