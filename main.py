import os
from PIL import Image

palka_h = 280

supported_types = ['jpg', 'jpeg', 'png', 'bmp', 'gif']
processed_path = os.path.dirname(__file__) + "/processed"
images_path = os.path.dirname(__file__) + "/images"
images = os.listdir(images_path)

def resize(image_name):
    image_path = '{}/{}'.format(images_path, image_name)
    print image_path
    im = Image.open(image_path, 'r').convert('RGB')

    w=im.size[0]
    h=im.size[1]

    print 'Original: {}, {}'.format(w, h)

    if h > palka_h:
        new_h = palka_h
        new_w = new_h * w / h
        #im = im.resize((new_w, new_h), Image.ANTIALIAS)
        im = im.resize((new_w, new_h), Image.NEAREST)
        print 'resized: {}, {}'.format(new_w,new_h)

    pix = im.load()

    w=im.size[0]
    h=im.size[1]

    print w, h

    with open('{}/{}.palka'.format(processed_path, image), 'w') as the_file:
        print the_file
        for i in range(w):
          for j in range(h):
            the_file.write('#%02x%02x%02x' % pix[i,j])
          the_file.write('\n')

if __name__ == '__main__':
    for image in images:
        for type in supported_types:
            if image.endswith(type):
                resize(image)