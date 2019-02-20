import os
from PIL import Image

palka_h = 267

supported_types = ['jpg', 'jpeg', 'png', 'bmp', 'gif']
processed_path = os.path.dirname(__file__) + "/processed"
images_path = os.path.dirname(__file__) + "/images"
images = os.listdir(images_path)

def resize(image_name):
    image_path = '{}/{}'.format(images_path, image_name)
    print("Image path: {}".format(image_path))

    im = Image.open(image_path, 'r')

    try:
      red, green, blue, alpha = im.split()
      alpha = im.split()[-1]
      im.putalpha(alpha)
    except Exception as err:
      print("Error: {}".format(err))

    w=im.size[0]
    h=im.size[1]

    print('Original size: {}, {}'.format(w, h))

    # Resize 
    if h > palka_h:
      new_h = palka_h
      new_w = new_h * w / h
      #im = im.resize((new_w, new_h), Image.ANTIALIAS)
      im = im.resize((new_w, new_h), Image.NEAREST)
      print("New size {}, {}".format(new_w, new_h))

    w=im.size[0]
    h=im.size[1]
    pix = im.load()

    # Save new image in 'palka' format
    palka_file = '{}/{}.palka'.format(processed_path, image)
    with open(palka_file, 'w') as the_file:
      print("Created palka file: {}".format(palka_file))
      for i in range(w):
        for j in range(h):
          try:
            r, g, b, a = pix[i,j]
          except:
            r, g, b = pix[i,j]
          the_file.write('#%02x%02x%02x' % (r, g, b))
          the_file.write('\n')

if __name__ == '__main__':
    for image in images:
        for image_type in supported_types:
            if image.endswith(image_type):
                resize(image)
