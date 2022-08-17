import os
import xml.dom.minidom as minidom

if __name__ == '__main__':
    dirlist = ['fonts', 'icons', 'jsons', 'qmls', 'scripts', 'styles']
    dom = minidom.getDOMImplementation().createDocument(None,'RCC',None)
    root = dom.documentElement
    rootElement = dom.createElement('qresource')
    # element.appendChild(dom.createTextNode('default'))
    rootElement.setAttribute('prefix', "/")
    root.appendChild(rootElement)
    for dirname in dirlist:
        if not os.path.exists(dirname):
            continue
        for file in os.listdir(dirname):
            element = dom.createElement('file')
            element.appendChild(dom.createTextNode(dirname + '/' + file))
            rootElement.appendChild(element)
    with open('resources.qrc', 'w', encoding='utf-8') as f:
        dom.writexml(f, addindent='    ', newl='\n', encoding='utf-8')

