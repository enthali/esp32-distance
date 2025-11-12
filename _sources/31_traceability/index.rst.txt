Traceability
============

This section contains automatically generated traceability information showing relationships between requirements, design specifications, and implementations.


Requirements Hierarchy Visualization
-------------------------------------

This diagram shows the hierarchical relationship between system-level and component-level requirements.

.. needflow::
   :filter: "REQ_SYS" in id or "REQ_WEB" in id or "REQ_CFG" in id or "REQ_NETIF" in id
   :show_link_names:
   :link_types: links
   :scale: 80%

System Requirements Flow
^^^^^^^^^^^^^^^^^^^^^^^^

System-level requirements and their component refinements:

.. needflow::
   :filter: "REQ_SYS" in id
   :show_link_names:
   :link_types: links


Web Server Component Traceability
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. needflow::
   :filter: "REQ_WEB" in id or id in ["REQ_SYS_WEB_1", "REQ_SYS_NET_1"]
   :show_link_names:
   :link_types: links
   :scale: 90%


Configuration Manager Traceability
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. needflow::
   :filter: "REQ_CFG" in id or id == "REQ_SYS_CFG_1"
   :show_link_names:
   :link_types: links
   :scale: 90%


Network Tunnel (QEMU) Traceability
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. needflow::
   :filter: "REQ_NETIF" in id or id in ["REQ_SYS_SIM_1", "REQ_SYS_NET_1"]
   :show_link_names:
   :link_types: links
   :scale: 90%


Requirements Tables
-------------------

All System Requirements
^^^^^^^^^^^^^^^^^^^^^^^

.. needtable::
   :filter: "REQ_SYS" in id
   :columns: id, title, status, priority, tags
   :style: table

All Component Requirements
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Web Server Requirements
"""""""""""""""""""""""

.. needtable::
   :filter: "REQ_WEB" in id
   :columns: id, title, status, priority, links
   :style: table

Configuration Manager Requirements
"""""""""""""""""""""""""""""""""""

.. needtable::
   :filter: "REQ_CFG" in id
   :columns: id, title, status, priority, links
   :style: table

Network Tunnel Requirements
""""""""""""""""""""""""""""

.. needtable::
   :filter: "REQ_NETIF" in id
   :columns: id, title, status, priority, links
   :style: table


Requirements Statistics
-----------------------

.. note::
   Visual statistics (pie charts) require Matplotlib. 
   Install with: ``pip install sphinx-needs[plotting]``

Status Distribution (Table)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. needtable::
   :columns: status, id
   :style: table
   :colwidths: 20, 80

Priority Distribution (Table)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. needtable::
   :columns: priority, id
   :style: table
   :colwidths: 20, 80


Coverage Analysis
-----------------

Requirements by Status
^^^^^^^^^^^^^^^^^^^^^^

.. needtable::
   :columns: status, id, title
   :style: table

All Requirements List
^^^^^^^^^^^^^^^^^^^^^

Complete list of all requirements with status and links:

.. needlist::
   :show_status:
   :show_tags:
