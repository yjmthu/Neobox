import os, json
import xml.dom.minidom as minidom

if __name__ == '__main__':
    dirlist = ['fonts', 'icons', 'jsons', 'scripts', 'styles']
    dom = minidom.getDOMImplementation()
    if not dom: exit(0)
    doc = dom.createDocument(None,'RCC',None)
    root = doc.documentElement
    rootElement = doc.createElement('qresource')
    # element.appendChild(dom.createTextNode('default'))
    rootElement.setAttribute('prefix', "/")
    root.appendChild(rootElement)
    for dirname in dirlist:
        if not os.path.exists(dirname):
            continue
        for file in os.listdir(dirname):
            element = doc.createElement('file')
            element.appendChild(doc.createTextNode(dirname + '/' + file))
            rootElement.appendChild(element)
    with open('resources.qrc', 'w', encoding='utf-8') as f:
        doc.writexml(f, addindent='    ', newl='\n', encoding='utf-8')
    with open("jsons/resources.json", "w", encoding="utf-8") as f:
        json.dump({
            "icons": ["icons/" + i for i in os.listdir("icons")],
            "styles": ["styles/" + i for i in os.listdir("styles")]
            }, f)

