import os 
import numpy as np
import matplotlib.pyplot as plt
import nibabel as nib

from nibabel.testing import data_path
from sklearn.preprocessing import normalize

#Import the CT scan using nibabel
example_filename = os.path.join(data_path, '1_t0_ct.nii')
scan = nib.load('1_t0_ct.nii')
img = scan.get_fdata()

#Normalize the data
img_max = np.max(img)
img_min = np.min(img)

img_normalized = (img - img_min) / (img_max - img_min)

#Consider segmenting the data with a simple cutoff
threshold = 0.263
img_segmented = np.where(img_normalized > threshold, img_normalized, 0.0)

#Save as a binary
img_rescaled = (img_segmented * 255).astype(np.uint8)


#Show result
plt.imshow(img_rescaled[:,:,170], aspect='equal') 
plt.show()

img_opengl_format = img_rescaled.transpose((2,1,0))

print(img_opengl_format.shape)

img_opengl_format.tofile("ct_scan.raw")