## Instructions for setup


1. Obtain NIFTI files from a source like:

https://www.cancerimagingarchive.net/collection/ucsd-ptgbm/ (I think I used this one for the skull)
	
2. Download them using IBM Aspera Connect Plugin

2. Inside the python folder run these commands: 

~~~bash
python -m venv venv 

venv\Scripts\Activate 

pip install -r requirements.txt
~~~

4. To generate .raw (assuming virtual env open), run:

~~~bash
python medical_imaging_loader.py
~~~