from pathlib import Path
import datetime
import shutil



data_dir = Path(__file__).parent / "data"
tstring = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M')
transfer_dir = data_dir / f"transfer_{tstring}"


card_dir = Path("D:\\")

for pat in ['serialLog*', 'dataLog*']:
    type_dir = transfer_dir / pat[:-1]
    created=False
    for file in card_dir.glob(pat):
        if not created:
            created = True
            type_dir.mkdir(parents=True, exist_ok=True)
            
        print(file.name)
        try:
            shutil.move(file, type_dir / file.name)
        except OSError:
            pass
    


    
