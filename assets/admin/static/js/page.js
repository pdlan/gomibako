var delete_id = '';
var edit_id = '';

function new_publish() {
    if ($('#new-title').val() == '') {
        $('#new-title-group').addClass('has-error');
        return;
    }
    $.post('/admin/page/new', JSON.stringify({
        'title': $('#new-title').val(),
        'content': $('#new-content').summernote('code'),
        'order': $('#new-order').val()
    }), function(data, status, xhr) {
        window.location.href = '/admin/page';
        return;
    }, 'text');
}

function edit_submit() {
    if ($('#edit-title').val() == '') {
        $('#edit-title-group').addClass('has-error');
        return;
    }
    $.post('/admin/page/edit', JSON.stringify({
        'id': edit_id,
        'title': $('#edit-title').val(),
        'content': $('#edit-content').summernote('code'),
        'order': $('#edit-order').val()
    }), function(data, status, xhr) {
        window.location.href = '/admin/page';
        return;
    }, 'text');
}

function delete_page() {
    $.get('/admin/page/delete/' + delete_id, function(data, status) {
        if (data != 'ok') {
            $('#alert-failure').css('display', 'block');
        } else {
            location.reload(true);
        }
    });
}

function edit_show() {
    var id_encoded = $(this).attr('data-id');
    var id = decodeURI(id_encoded);
    $.get('/admin/page/json/' + id_encoded, function(data, status) {
        $('#edit-order').val(data.order);
        $('#edit-title').val(data.title);
        $('#edit-content').summernote('code', data.content);
    });
    $('#modal-edit-page').modal('show');
    edit_id = id;
}

$(document).ready(function() {
    $('#new-publish').click(new_publish);
    $('#edit-submit').click(edit_submit);
    $('.delete-page').click(function() {
        delete_id = decodeURI($(this).attr('data-id'));
        $('#alert-failure').css('display', 'none');
        $('#modal-delete').modal('show');
    });
    $('.edit-page').click(edit_show);
    $('#new-content').summernote({
        height: 300
    });
    $('#edit-content').summernote({
        height: 300
    });
    $('#delete-page').click(delete_page);
});